#pragma once

#include <string_view>


// String split iterator
namespace string_view {
  namespace impl {
    template<typename SplitByType>
    struct StringViewSplit {
      StringViewSplit(std::string_view source, SplitByType splitBy) : source(source), splitBy(splitBy) {}

      struct iterator {
        using element_type = std::string_view;
        using reference_type = const std::string_view&;
        using pointer_type = const std::string_view*;
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;

        iterator(std::string_view source, SplitByType splitBy) : source(source), splitBy(splitBy) { 
          ++(*this); // position onto first split
        }

        iterator& operator++() {
          if (separatorEndPos == std::string_view::npos) {
            completed = true;
            return *this;
          }

          auto pos = source.find(splitBy, separatorEndPos);
          if (pos == std::string_view::npos) {
            // No more separators found
            subMatch = source.substr(separatorEndPos);
            separatorEndPos = pos;
          } else {
            // Found another separator
            subMatch = source.substr(separatorEndPos, pos - separatorEndPos);
            separatorEndPos = pos;
            if constexpr (std::is_same_v<SplitByType, char>) {
              ++separatorEndPos; // increment past the separator char
            } else {
              separatorEndPos += std::string_view(splitBy).size(); // string_view or char* separator
            }
          }

          return *this;
        }

        element_type operator*() const { return subMatch; }

        struct sentinel {};

        bool operator==(sentinel) const { return completed; }
        bool operator!=(sentinel) const { return !completed; }

        std::string_view source;
        std::string_view subMatch;
        SplitByType splitBy;
        size_t separatorEndPos = 0;
        bool completed = false;
      };

      auto begin() const { return iterator(source, splitBy); }
      auto end() const { return iterator::sentinel(); }

      SplitByType splitBy;
      std::string_view source;
    };
  }


  auto split(std::string_view source, char splitChar) {
    return impl::StringViewSplit<char>(source, splitChar);
  }

  auto split(std::string_view source, std::string_view splitBy) {
    return impl::StringViewSplit<std::string_view>(source, splitBy);
  }
}


