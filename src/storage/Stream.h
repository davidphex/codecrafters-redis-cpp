//
// Created by david on 11/27/26.
//

#ifndef REDIS_STARTER_CPP_STREAM_H
#define REDIS_STARTER_CPP_STREAM_H
#include <array>
#include <charconv>
#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

using Value = std::variant<int64_t, double, bool, std::string>;

using Field = std::pair<std::string, Value>;

struct StreamID {
    uint64_t ms = 0;
    uint64_t seq = 0;

    // Generates ==, !=, <, <=, >, >= automatically (compares ms first, then seq)
    auto operator<=>(const StreamID&) const = default;

    std::string to_string() const {
        return std::to_string(ms) + "-" + std::to_string(seq);
    }

    // Fast, allocation-free parser for strings like "0-1" or "1700000000-5"
    static std::optional<StreamID> parse(std::string_view str) {
        auto dash = str.find('-');
        if (dash == std::string_view::npos) {
            return std::nullopt;
        }

        StreamID id;
        auto [p1, ec1] = std::from_chars(str.data(), str.data() + dash, id.ms);
        auto [p2, ec2] = std::from_chars(str.data() + dash + 1, str.data() + str.size(), id.seq);

        if (ec1 != std::errc{} || p1 != str.data() + dash ||
            ec2 != std::errc{} || p2 != str.data() + str.size()) {
            return std::nullopt;
            }
        return id;
    }
};

struct Entry {
    StreamID id;
    std::vector<Field> fields;
};

struct Chunk {
    static constexpr int CAPACITY = 128;
    std::vector<Entry> entries;

    Chunk() {
        entries.reserve(CAPACITY);
    }

    [[nodiscard]] bool is_full() const {
        return entries.size() >= CAPACITY;
    }
};

struct ChunkMeta {
    StreamID first_id;
    StreamID last_id;
    std::unique_ptr<Chunk> chunk;
};


class Stream {
public:
    bool append(StreamID id, std::vector<Field> fields);
    std::vector<const Entry*> range(StreamID start_id, StreamID stop_id);
private:
    std::deque<ChunkMeta> chunks_;
};


#endif //REDIS_STARTER_CPP_STREAM_H
