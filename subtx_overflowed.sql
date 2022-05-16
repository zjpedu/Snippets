-- 当 exception 执行了,到底发生了什么,如果执行了,我们看到数据实际上只插入一条,并且也没有发生子事务溢出的情况
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


select wait_event_type, wait_event, state, query  from pg_stat_activity where state='active';


-- 下面所有的方法都能复现子事务溢出的问题
-- 在 QE 上能够打出来 pid,不过目前只能 gdb attach 到 seg 上才能看到
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


-- 全部插入到 seg0 上,这个可以作为 QE 上的测试用例
CREATE TABLE t(c1 int);
CREATE OR REPLACE FUNCTION insert_data()
RETURNS VOID
LANGUAGE plpgsql
AS $$
DECLARE i int;
BEGIN
	FOR i in 0..300
	LOOP
		BEGIN
			INSERT INTO t(c1) VALUES(1);
		EXCEPTION
		WHEN UNIQUE_VIOLATION THEN
			NULL;	
		END;
	END LOOP;
END
$$;

BEGIN;
SELECT insert_data();
end;

-- QE 上的 demo
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
select count(*) from (select gp_subtrx_overflowed_pid()) AS test;  -- 所以我的测试用例只需要让它返回 1 个 pid 就认为是正确的,目前的方案是这样子的
end;


CREATE PROCEDURE transaction_test1()
LANGUAGE plpgsql
AS $$
begin
	for i in 0..300
	loop
		begin
			create temp table tmptab(c int);
			drop table tmptab;
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
select count(*) from (select gp_subtrx_overflowed_pid()) AS test;  -- 所以我的测试用例只需要让它返回 1 个 pid 就认为是正确的,目前的方案是这样子的
end;


-- 将下面脚本保存为 test.sh 生成的查询能够作为目前的测试用例
echo "begin;" > tx.sql
for i in {1..500}; do
  echo "insert into test values(1);" >> tx.sql
  echo "savepoint savepoint_${i};" >> tx.sql
done
\i tx.sql -- 此时在 QE 上发生了子事务溢出
