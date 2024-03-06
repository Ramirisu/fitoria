include(CheckCXXSourceCompiles)

macro(detect_std_format)
  check_cxx_source_compiles(
    "#include <version>
    #if defined(__cpp_lib_format) && __has_include(<format>)
    #include <format>
    #endif
    int main() {
        std::format(\"x\");
        return 0;
    }"
    FITORIA_CXX_COMPILER_HAS_STD_FORMAT)
endmacro()
