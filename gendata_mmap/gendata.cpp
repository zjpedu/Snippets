/**
 * 以内存映射文件的方式生成并读取数据
 */
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

extern "C" {
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
} // extern "C"

// generate float values in the range [0, floatrange]
static constexpr int floatrange = 100;
// the precision of the float values, 100 means 0.00, 1000 means 0.000
static constexpr int floatscale = 100;

static constexpr const char * attnames[] = {
        "c1", "c2", "c3", "c4",
};

static constexpr const char *datadir = "data/";
static constexpr const int ncolumns = 4;;
static constexpr int batchsize = 1000;
static constexpr int nbatches = 1000 * 100;
static constexpr int64_t interval = 1000000;

static int
generate_base()
{
    return std::rand() % (floatrange * floatscale);
}

static int
generate_delta()
{
    return std::rand() % floatrange - floatrange / 2;
}

static inline std::string
get_filepath(const std::string filename) {
    std::string fullname = std::string(datadir) + filename;
    return fullname;
}

// 生成数据
int generateData(void)
{
    mkdir(datadir, 0755);  // 创建文件目录

#pragma omp parallel for
    for (int col = 0; col < ncolumns; ++col) {
        std::string filename = get_filepath(attnames[col]);
        auto f = std::fopen(filename.c_str(), "wb");

        for (int batch = 0; batch < nbatches; ++batch) {
            if (col == 0) {
                std::vector<std::int32_t> values(batchsize, batch);
                std::fwrite(values.data(), sizeof(int32_t), batchsize, f);
            } else if (col == 1) {
                std::vector<std::int64_t> values(batchsize);
                for (int i = 0; i < batchsize; ++i)
                    values[i] = interval * (i + 1);
                std::fwrite(values.data(), sizeof(int64_t), batchsize, f);
            } else if (col == 2) {
                std::vector<float> values(batchsize);
                auto base = generate_base();
                for (auto &v: values) {
                    base += generate_delta();
                    v = float (base) / floatscale;
                }
                std::fwrite(values.data(), sizeof(float), batchsize, f);
            } else if (col == 3) {
                std::vector<double> values(batchsize);
                auto base = generate_base();
                for (auto &v: values) {
                    base += generate_delta();
                    v = double(base) / floatscale;
                }
                std::fwrite(values.data(), sizeof(double), batchsize, f);
            }
        }

        std::fclose(f);
    }

    return 0;
}

// 以内存映射文件的方式读取数据
struct File {
    int fd;
    const char *data;
    std::size_t datasize;

    File(const std::string name) {
        std::string filename = std::string("data/") + name;

        fd = ::open(filename.c_str(), O_RDONLY);
        assert(fd >= 0);

        datasize = ::lseek(fd, 0, SEEK_END);
        assert(datasize >= 0);

        auto ptr = ::mmap(nullptr, datasize, PROT_READ, MAP_PRIVATE, fd, 0);
        assert(ptr);

        data = static_cast<char *>(ptr);
    }

    ~File() {
        if (data)
            ::munmap(const_cast<char *>(data), datasize);

        if (fd >= 0)
            ::close(fd);
    }
};

int main(void) {
    // 生成数据
    generateData();

    //建立内存映射文件指针
    std::vector<std::unique_ptr<File>> files;
    for(int i = 0; i < sizeof(attnames)/ sizeof(attnames[0]); ++i)
        files.push_back(std::make_unique<File>(attnames[i]));

    // 从内存中读取文件数据
    for(int i = 0; i < files.size(); ++i){
        const void* data = static_cast<const void *> (files[i].get()->data);
        if(i == 0){
            auto tmp = static_cast<const int32_t *>(data);
            for(int j = 0; j < 1000; j++){
                //输出一下 c1 文件的前 1000 行
                std::cout << tmp[i] << "";
            }
            std::cout << std::endl;
        }
        if(i == 1){
            auto tmp = static_cast<const int64_t *>(data);
            for(int j = 0; j < 1000; j++){
                //输出一下 c2 文件的前 1000 行
                std::cout << tmp[i] << "";
            }
            std::cout << std::endl;
        }
        if(i == 2){
            auto tmp = static_cast<const float *>(data);
            for(int j = 0; j < 1000; j++){
                //输出一下 c1 文件的前 1000 行
                std::cout << tmp[i] << "";
            }
            std::cout << std::endl;
        }
        if(i == 3){
            auto tmp = static_cast<const double *>(data);
            for(int j = 0; j < 1000; j++){
                //输出一下 c1 文件的前 1000 行
                std::cout << tmp[i] << "";
            }
            std::cout << std::endl;
        }
    }
}
