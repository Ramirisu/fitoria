include(CheckCXXSourceCompiles)

macro(detect_std_format)
  check_cxx_source_compiles(
    "#include <format>
    int main() {
        std::format(\"x\");
        return 0;
    }"
    FITORIA_CXX_COMPILER_HAS_STD_FORMAT)
endmacro()
