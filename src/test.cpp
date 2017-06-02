
#include "shrinkwrap/xzbuf.hpp"
#include "shrinkwrap/gzbuf.hpp"
#include <fstream>

#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <random>
#include <chrono>
#include <iterator>
#include <sstream>
#include <limits>
#include <shrinkwrap/gzbuf.hpp>

namespace sw = shrinkwrap;

template <typename InT, typename OutT>
class test_base
{
public:
  test_base(const std::string& file_path, std::size_t block_size = std::numeric_limits<std::size_t>::max()):
    file_(file_path),
    block_size_(block_size)
  {
  }
protected:
  static bool file_exists(const std::string& file_path)
  {
    struct stat st;
    return (stat(file_path.c_str(), &st) == 0);
  }

  static bool generate_test_file(const std::string& file_path, std::size_t block_size)
  {
    OutT ofs(file_path);
    for (std::size_t i = 0; i < (2048 / 4) && ofs.good(); ++i)
    {
      if (((i * 4) % block_size) == 0)
        ofs.flush();
      ofs << std::setfill('0') << std::setw(3) << i << " " ;
    }
    return ofs.good();
  }
protected:
  std::string file_;
  std::size_t block_size_;
};

template <typename InT, typename OutT>
class iterator_test: public test_base<InT, OutT>
{
public:
  using test_base<InT, OutT>::test_base;
  bool operator()()
  {
    bool ret = false;

    if ((test_base<InT, OutT>::file_exists(this->file_) &&  std::remove(this->file_.c_str()) != 0) || !test_base<InT, OutT>::generate_test_file(this->file_, this->block_size_))
    {
      std::cerr << "FAILED to generate test file." << std::endl;
    }
    else
    {
      if (!run(this->file_))
      {
        std::cerr << "FAILED iterator test." << std::endl;
      }
      else
      {
        ret = true;
      }
    }

    return ret;
  }
private:
  bool run(const std::string& file_path)
  {
    InT is(file_path);
    std::istreambuf_iterator<char> it(is.rdbuf());
    std::istreambuf_iterator<char> end{};

    std::size_t integer = 0;
    for (std::size_t i = 0; it != end; ++i)
    {
      std::stringstream padded_integer;
      for (std::size_t j = 0; j < 4 && it != end; ++j,++it)
        padded_integer.put(*it);
      padded_integer >> integer;

      if (i != integer)
        return false;
    }

    return (integer == 511);
  }
};

template <typename InT, typename OutT>
class seek_test : public test_base<InT, OutT>
{
public:
  using test_base<InT, OutT>::test_base;

  bool operator()()
  {
    bool ret = false;

    if ((test_base<InT, OutT>::file_exists(this->file_) && std::remove(this->file_.c_str()) != 0) || !test_base<InT, OutT>::generate_test_file(this->file_, this->block_size_))
    {
      std::cerr << "FAILED to generate test file" << std::endl;
    }
    else
    {
      if (!run(this->file_))
      {
        std::cerr << "FAILED seek test." << std::endl;
      }
      else
      {
        ret = true;
      }
    }

    return ret;
  }
private:
  static bool run(const std::string& file_path)
  {
    InT ifs(file_path);
    std::vector<int> pos_sequence;
    pos_sequence.reserve(128);
    std::mt19937 rg(std::uint32_t(std::chrono::system_clock::now().time_since_epoch().count()));
    for (unsigned i = 0; i < 128 && ifs.good(); ++i)
    {
      int val = 2048 / 4;
      int pos = rg() % val;
      pos_sequence.push_back(pos);
      ifs.seekg(pos * 4, std::ios::beg);
      ifs >> val;
      if (val != pos)
      {
        std::cerr << "Seek failure sequence:" << std::endl;
        for (auto it = pos_sequence.begin(); it != pos_sequence.end(); ++it)
        {
          if (it != pos_sequence.begin())
            std::cerr << ",";
          std::cerr << *it;
        }
        std::cerr << std::endl;
        return false;
      }
    }
    return ifs.good();
  }
};

template <typename InT, typename OutT>
class virtual_offset_seek_test : public test_base<InT, OutT>
{
public:
  using test_base<InT, OutT>::test_base;

  bool operator()()
  {
    bool ret = false;

    if ((test_base<InT, OutT>::file_exists(this->file_) && std::remove(this->file_.c_str()) != 0) || !test_base<InT, OutT>::generate_test_file(this->file_, this->block_size_))
    {
      std::cerr << "FAILED to generate test file" << std::endl;
    }
    else
    {
      if (!run(this->file_))
      {
        std::cerr << "FAILED seek test." << std::endl;
      }
      else
      {
        ret = true;
      }
    }

    return ret;
  }
private:
  static bool run(const std::string& file_path)
  {
    InT ifs(file_path);
    std::vector<int> pos_sequence;
    pos_sequence.reserve(128);
    std::mt19937 rg(std::uint32_t(std::chrono::system_clock::now().time_since_epoch().count()));
    for (unsigned i = 0; i < 128 && ifs.good(); ++i)
    {
      int val = 2048 / 4;
      int pos = rg() % val;
      pos_sequence.push_back(pos);

      std::uint64_t virtual_offset{};
      {
        InT idx_ifs(file_path);
        virtual_offset = idx_ifs.tellg();
        int tmp;
        while (idx_ifs >> tmp)
        {
          if (tmp == pos)
            break;
          virtual_offset = idx_ifs.tellg();
        }
      }

      ifs.seekg(virtual_offset);
      ifs >> val;
      if (val != pos)
      {
        std::cerr << "Seek failure sequence:" << std::endl;
        for (auto it = pos_sequence.begin(); it != pos_sequence.end(); ++it)
        {
          if (it != pos_sequence.begin())
            std::cerr << ",";
          std::cerr << *it;
        }
        std::cerr << std::endl;
        return false;
      }
    }
    return ifs.good();
  }
};

int main(int argc, char* argv[])
{
  int ret = -1;

  if (argc > 1)
  {
    std::string sub_command = argv[1];
    if (sub_command == "xzseek")
      ret = !(seek_test<sw::ixzstream, sw::oxzstream>("test_seek_file.txt.xz")());
    else if (sub_command == "xziter")
      ret = !(iterator_test<sw::ixzstream, sw::oxzstream>("test_iterator_file.txt.xz")() && iterator_test<sw::ixzstream, sw::oxzstream>("test_iterator_file_512.txt.xz", 512)() && iterator_test<sw::ixzstream, sw::oxzstream>("test_iterator_file_1024.txt.xz", 1024)());
    else if (sub_command == "gziter")
      ret = !(iterator_test<sw::igzstream, sw::ogzstream>("test_iterator_file.txt.gz")() && iterator_test<sw::igzstream, sw::ogzstream>("test_iterator_file_512.txt.gz", 512)() && iterator_test<sw::igzstream, sw::ogzstream>("test_iterator_file_1024.txt.gz", 1024)());
    else if (sub_command == "bgzseek")
      ret = !(virtual_offset_seek_test<sw::ibgzstream, sw::obgzstream>("test_seek_file.txt.bgzf", 512)());
    else if (sub_command == "bgziter")
      ret = !(iterator_test<sw::ibgzstream, sw::obgzstream>("test_iterator_file.txt.bgzf")() && iterator_test<sw::ibgzstream, sw::obgzstream>("test_iterator_file_512.txt.bgzf", 512)() && iterator_test<sw::ibgzstream, sw::obgzstream>("test_iterator_file_1024.txt.bgzf", 1024)());
  }

  return ret;
}
