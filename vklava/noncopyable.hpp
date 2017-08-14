#pragma once

class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(NonCopyable const&) = delete;
    NonCopyable &operator=(NonCopyable const&) = delete;
};

