#pragma once

#include <iostream>
#include <string>
#include <ranges>

namespace stream {
  namespace impl {
    struct LineIterator {
      using element_type = std::string;
      using reference = const std::string&;
      using pointer = const std::string*;
      using iterator_category = std::input_iterator_tag;
      using iterator_concept = std::input_iterator_tag;
      using difference_type = ptrdiff_t;

      LineIterator(std::istream& stream) : stream(&stream) { std::getline(stream, line); }
      LineIterator(LineIterator&& other) : stream(other.stream), line(std::move(other.line)) { other.stream = nullptr; }
      LineIterator& operator=(LineIterator&& other) {
        stream = other.stream;
        other.stream = nullptr;
        line = std::move(other.line);
        return *this;
      }

      struct Sentinel {};


      LineIterator& operator++() { std::getline(*stream, line); return *this;  }

      bool operator==(Sentinel) const { return !stream || !*stream; }
      bool operator!=(Sentinel) const { return stream && *stream; }

      reference operator*() const { return line; }
      pointer operator->() const { return &line; }

    private:
      std::string line;
      std::istream* stream = nullptr;
    };

    struct Lines {
      Lines(std::istream& inputStream) : inputStream(inputStream) {}

      auto begin() { return LineIterator(inputStream); }
      auto end() { return LineIterator::Sentinel(); }

      std::istream& inputStream;
    };
  }

  /** Utility function to simply iterate over all lines of a stream/file
   */
  auto lines(std::istream& inputStream) {
    return impl::Lines(inputStream);
  }
}