#### 子事务问题引入

子事务又称为“嵌套事务”,它指的是在一个开始的事务内存开启其它的事务,有三种方式开启子事务:
* savepoint 显式开启子事务
* begin...exception...end 包含有 exception 的语句块会隐式开启子事务
* plpython.subtransaction()/plpy.execute() 也会开启子事务

下面的 insert 都可以换成 update 这样子的修改操作.修改操作会分配 xid,这些信息需要被系统追踪.

We want understand better what happens when we exceed 64 **active** subtransactions 
in a GPDB cluster, but the active savepoints are being held by a relatively 
high number of long running transactions.

```sql
create table test(c int PRIMARY KEY);
CREATE PROCEDURE transaction_test2()
LANGUAGE plpgsql
AS $$
DECLARE i int;
begin
	for i in 0..300
	loop
		begin
			insert into test(c) values(1);
		exception
			WHEN UNIQUE_VIOLATION THEN
         		NULL;  -- ignore the error
        end;
    end loop;
end;
$$;

begin;
call transaction_test2();
end;
```

正如上面代码所示,当 exception 执行了,到底发生了什么?
如果执行上述代码,我们看到数据实际上只插入一条?并且也没有发生子事务溢出的情况

```sql
select gp_segment_id, * from test;  -- 只返回一条数据
```

当查询每次进入 begin...exception...end 这样的查询中时,就会开启一个子事务,
当查询正常走出该语句块时,子事务正常提交,如果查询走入 exception 部分,则 rollback.
** 一定要注意 > 64 指的是处于 commited 状态的子事务数量大于 64 个时,才会将其标记为子事务溢出.**
但是无论是否标记子事务溢出, xid 都会被消耗,只要是修改操作,即便失败了也会消耗 xid.

> A snapshot is initialized by looking at the process array, which is stored in 
> shared memory and contains information about all currently running backends. 
> This, of course, contains the current transaction ID of the backend and has 
> room for at most 64 non-aborted subtransactions per session. 
> If there are more than 64 such subtransactions, the snapshot is marked 
> as suboverflowed.

#### 子事务溢出问题复现

```sql
select wait_event_type, wait_event, state, query  
from pg_stat_activity where state='active';
```

-- 下面所有的方法都能复现子事务溢出的问题
-- 在 QE 上能够打出来 pid,不过目前只能 gdb attach 到 seg 上才能看到

```sql
create language plpython3u;
create table t(a int);
create function f() returns void as
$$
for i in range(300):
    plpy.execute("insert into t values (1);")
$$
language plpython3u;

begin;
select f();
end;
```

-- 全部插入到 seg0 上,这个可以作为 QE 上的测试用例
```sql
s

BEGIN;
SELECT insert_data();
end;
```

-- QE 上的 demo
```sql
create table test(c int);
CREATE PROCEDURE transaction_test2()
LANGUAGE plpgsql
AS $$
DECLARE i int;
begin
	for i in 0..300
	loop
		begin
			insert into test(c) values(1);
		exception
			WHEN others THEN
         		NULL;  -- ignore the error
         end;
    end loop;
end;
$$;

begin;
call transaction_test2();
end;
```

```sql
-- plpython
create language plpython3u;
create function ftmp() returns void as
$$
for i in range(300):
    plpy.execute("create temp table tmptab(c int);");
    plpy.execute("drop table tmptab;");
$$
language plpython3u;

begin;
select ftmp();
-- select gp_subtrx_overflowed_pid();  -- 在 QD 上能够打印出对应的 pid
-- 所以我的测试用例只需要让它返回 1 个 pid 就认为是正确的,目前的方案是这样子的
select count(*) from (select gp_subtrx_overflowed_pid()) AS test;  
end;
```

```sql
CREATE PROCEDURE transaction_test1()
LANGUAGE plpgsql
AS $$
begin
	for i in 0..300
	loop
		begin
			create temp table tmptab(c int);
			drop table tmptab;
			-- insert into t1 values(1);
		exception
			WHEN others THEN
         		NULL;  -- ignore the error
        end;
    end loop;
end;
$$;
begin;
call transaction_test1();
-- select gp_subtrx_overflowed_pid();  -- 在 QD 上能够打印出对应的 pid
select count(*) from (select gp_subtrx_overflowed_pid()) AS test;
end;
```

-- 将下面脚本保存为 test.sh 生成的查询能够作为目前的测试用例
```shell
echo "begin;" > tx.sql
for i in {1..500}; do
  echo "insert into test values(1);" >> tx.sql
  echo "savepoint savepoint_${i};" >> tx.sql
done
```
```sql
\i tx.sql -- 此时在 QE 上发生了子事务溢出
```

-- 复现 Dell 问题一个比较关键的问题是:
-- 更新互相不冲突,但是 select 会互相冲突


#### 参考资料
1. https://www.cybertec-postgresql.com/en/subtransactions-and-performance-in-postgresql/
2. https://github.com/apache/madlib/blob/4987e8fe5367bb823afb1bd4020fd6f0fa603258/src/ports/postgres/modules/graph/wcc.py_in#L63
这个函数中包含了大量的 plpy.execute("create temp table ...") 这会产生大量的子事务
3. SimpleLruReadPage_ReadOnly ← SubTransGetData ← SubTransGetTopmostTransaction ← XidInMVCCSnapshot_Local 这个调用栈产生的原因是子事务溢出
4. https://fluca1978.github.io/2020/02/05/PLPGSQLExceptions.html 官方文档对 begin...exception...end 没有很好的描述,这篇文章补足了这一点
5. https://postgres.ai/blog/20210831-postgresql-subtransactions-considered-harmful#problem-2-per-session-cache-overflow 这个链接包含了大量子事务相关的问题和源码文件
6. https://buttondown.email/nelhage/archive/notes-on-some-postgresql-implementation-details/ 里面讲解了 MultiXact 问题,这表明 select ... for share 的查询可能导致性能问题
本文提出,从未在你的查询中使用 SELECT FOR SHARE 因为这可能导致很难排查的性能下降
7. https://www.postgresql.org/docs/12/plpgsql-control-structures.html#PLPGSQL-ERROR-TRAPPING 官方介绍 exception 的文档,不过没什么用,没有讲明白
8. https://about.gitlab.com/blog/2021/09/29/why-we-spent-the-last-month-eliminating-postgresql-subtransactions/ 有详细的原因调查过程,非常棒的子事务溢出资料
9. 复现方法和 patch https://www.postgresql.org/message-id/003201d79d7b$189141f0$49b3c5d0$@tju.edu.cn

Feat: Find the pids of overflowd subtransaction

In gpdb, it support using `savepoint`, `begin...exception...`, and `plpython`
to issue the subtransactions. Especially, it may be much serious for the
application error handling. It may issue many more subtransaction for
error handling.

For the postgres and gpdb, the number of active subtransactions has maximum
value. The subtransaction state move back and force when the number of
active subtransactions exceed the maximum value (64) for inspect the
snapshot visiblity. It may cause shake in system performance.

The DBA need to inspect the cause. So, we add the helper function for
finding the pids of overflowed subtransaction for coordinator and segments.
