//
// Created by david on 11/27/26.
//

#include "Stream.h"

#include <algorithm>

bool Stream::append(StreamID id, std::vector<Field> fields) {
    if (!chunks_.empty() && id <= chunks_.back().first_id) {
        return false;
    }

    if (chunks_.empty() || chunks_.back().chunk->is_full()) {
        auto new_chunk = std::make_unique<Chunk>();
        chunks_.push_back(ChunkMeta{
            .first_id = id,
            .last_id = id,
            .chunk = std::move(new_chunk)
        });
        return true;
    }

    auto &tail = chunks_.back();
    tail.chunk->entries.emplace_back(id, std::move(fields));
    tail.last_id = id;
    return true;
}

std::vector<const Entry*> Stream::range(StreamID start_id, StreamID stop_id) {
    std::vector<const Entry*> result;

    if (chunks_.empty() || start_id > stop_id) {
        return result;
    }

    // 1. Binary search for the first chunk that could contain start_id (last_id >= start_id)
    auto chunk_it = std::lower_bound(
        chunks_.begin(), chunks_.end(), start_id,
        [](const ChunkMeta& meta, StreamID id) {
            return meta.last_id < id;
        }
    );

    // 2. Scan across relevant chunks
    for (; chunk_it != chunks_.end(); ++chunk_it) {
        // Stop immediately if this entire chunk starts after end_id
        if (chunk_it->first_id > stop_id) {
            break;
        }

        const auto& entries = chunk_it->chunk->entries;

        // Binary search inside the chunk (only needed for the first chunk, O(1) for the rest)
        auto entry_it = std::lower_bound(
            entries.begin(), entries.end(), start_id,
            [](const Entry& e, StreamID id) {
                return e.id < id;
            }
        );

        for (; entry_it != entries.end(); ++entry_it) {
            if (entry_it->id > stop_id) {
                return result;
            }
            result.push_back(&(*entry_it));
        }
    }

    return result;

}
