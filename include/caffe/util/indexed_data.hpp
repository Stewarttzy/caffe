#ifndef CAFFE_UTIL_INDEXED_DATA_H_
#define CAFFE_UTIL_INDEXED_DATA_H_

#include <string>
#include <vector>

#include "caffe/common.hpp"
#include "caffe/proto/caffe.pb.h"

namespace caffe {

typedef uint32_t index_type;

/**
 * @brief An abstract class for retrieving data array
 *        by index. Used by IndirectionLayer to convert indices
 *        into blobs.
 */
template <typename Dtype>
class IndexedDataReader {
 public:
  IndexedDataReader() {}
  virtual ~IndexedDataReader() {}

  /**
   * @brief Retrieve the data.
   * @param index The index of the data
   * @param out The caller allocated storage to write data into
   * @param length The length of the caller allocated array
   * @return The actual length of the data, which could be larger or
   *         smaller than the length parameter
   *
   * This function shall be stateless instead of stream-like. That is,
   * calling it twice with the same arguments shall return the same
   * data, provided the underlying storage do not mutate in the meantime.
   *
   * It is not marked const because implementations may cache responses.
   *
   * When length equals zero, out can be a null pointer; otherwise, a null
   * out will cause undefined behavior.
   */
  virtual index_type read(index_type index,
          Dtype* out, index_type length) = 0;

  /**
   * @brief Factory function for creating subtypes of IndexedDataReader
   */
  static shared_ptr<IndexedDataReader<Dtype> > make_reader(
      IndirectionParameter::IndirectionSourceType type,
      const std::string& source_file);

  DISABLE_COPY_AND_ASSIGN(IndexedDataReader);
};

/**
 * @brief A cache class of IndexedDataReader.
 */
template <typename Dtype>
class IndexedDataReadCache: public IndexedDataReader<Dtype> {
 private:
  shared_ptr<IndexedDataReader<Dtype> > reader_;
  index_type length_;

 public:
  /**
   * @brief Constructor
   * @param reader The underlying reader
   * @param The length of each data array
   *
   * The cache only works with readers whose data array has the same
   * length, and has no gaps in indices
   */
  explicit IndexedDataReadCache(shared_ptr<IndexedDataReader<Dtype> >
          reader, index_type length): reader_(reader), length_(length)
  {}

  index_type data_length() const { return length_; }
};

template <typename Dtype>
class LinearIndexedStorage : public IndexedDataReader<Dtype> {
 protected:
  std::vector<Dtype> data_;
  std::vector<std::size_t> indices_;

 public:
  virtual index_type read(index_type index,
        Dtype* out, index_type length);
};

/**
 * @brief The simplest indexed data storage backed by a text file
 *        where each line consists of numbers separated by whitespace
 */
template <typename Dtype>
class SimpleIndexedTextFile
  : public LinearIndexedStorage<Dtype> {
 public:
    explicit SimpleIndexedTextFile(const std::string& source_file);
};

/**
 * @brief A indexed data storage where each line of source file points
 *        to a binary file of Dtype array in machine byte order
 */
template <typename Dtype>
class IndexedBinaryFiles : public IndexedDataReader<Dtype> {
 private:
  std::vector<std::string> file_names_;
 public:
  explicit IndexedBinaryFiles(const std::string& source_file);

  virtual index_type read(index_type index,
        Dtype* out, index_type length);
};
}  // namespace caffe

#endif    // CAFFE_UTIL_INDEXED_DATA_H_
