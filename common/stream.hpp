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
      LineIterator(const LineIterator& other) : stream(other.stream), line(other.line) {} // needed for post increment
      LineIterator(LineIterator&& other) : stream(other.stream), line(std::move(other.line)) { other.stream = nullptr; }
      LineIterator& operator=(LineIterator&& other) {
        stream = other.stream;
        other.stream = nullptr;
        line = std::move(other.line);
        return *this;
      }

      // Needed for std::ranges::begin() support
      LineIterator& operator=(const LineIterator& other) {
        stream = other.stream;
        line = other.line;
        return *this;
      }

      struct Sentinel {
        // Needed for std::ranges::end()
        bool operator==(const LineIterator& it) const { return it == *this; }
        bool operator!=(const LineIterator& it) const { return it != *this; }
      };


      LineIterator& operator++() { std::getline(*stream, line); return *this;  }
      // Needed for std::ranges::begin() support
      LineIterator operator++(int) { auto copy = *this; std::getline(*stream, line); return copy; }

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


    struct DefaultSeparator {};
    std::ostream& operator<<(std::ostream& out, DefaultSeparator) { return out << ','; }
  }

  /** Utility function to simply iterate over all lines of a stream/file
   */
  auto lines(std::istream& inputStream) {
    return impl::Lines(inputStream);
  }

  template<typename Rng, typename Sep = impl::DefaultSeparator, typename Proj = std::identity>
  std::ostream& joinInto(std::ostream& out, Rng&& range, Sep&& separator = {}, Proj projection = {}) {
    auto it = std::begin(range);
    auto end = std::end(range);
    while (it != end) {
      out << projection(*it);
      if (++it != end) {
        out << separator;
      }
    }

    return out;
  }

  template<typename Rng, typename Sep = impl::DefaultSeparator, typename Proj = std::identity>
  std::string join(Rng&& range, Sep&& separator = {}, Proj projection = {}) {
    std::ostringstream out;
    joinInto(out, std::forward<Rng>(range), std::forward<Sep>(separator), projection);
    return out.str();
  }

}