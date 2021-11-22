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


static constexpr const char *datadir = "data/";
static constexpr const int ncolumns = 1;
static constexpr int batchsize = 1000;
static constexpr int nbatches = 1000 * 100;

static inline std::string
get_filepath(const std::string filename) {
	std::string fullname = std::string(datadir) + filename;
	return fullname;
}

std::vector<int> size_array(batchsize * nbatches);
// 生成数据
int generateData(void)
{
	mkdir(datadir, 0755);  // 创建文件目录

#pragma omp parallel for
	for (int col = 0; col < ncolumns; ++col) {
		std::string filename = get_filepath("text");
		auto f = std::fopen(filename.c_str(), "wb");
		int count = 0;
		for (int i = 0; i < nbatches * batchsize; ++i) {
			std::string tmp = "text" + std::to_string(count);
			count++;
			size_array[i] = tmp.size();
			std::fwrite(tmp.c_str(), tmp.size(), 1, f);
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
	std::unique_ptr<File> file(std::make_unique<File>("text"));

	// 从内存中读取文件数据
	auto ptr = file.get()->data;
	for(int i = 0; i < nbatches * batchsize; ++i){
		char tmp[size_array[i]];
		int j = 0;
		for(; j < size_array[i]; j++)
			tmp[j] = *(ptr+j);
		tmp[j] = '\0';
		std::string str(tmp);
		ptr = ptr + size_array[i];
		std::cout<< str << std::endl;
	}
}
