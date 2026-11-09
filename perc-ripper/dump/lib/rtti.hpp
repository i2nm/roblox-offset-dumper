namespace rtti
{
    std::string get(uintptr_t objectAddress);
    std::unordered_map<std::string, uintptr_t> scan(
        const std::vector<std::string>& symbols
    );
}