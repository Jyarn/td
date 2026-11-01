#pragma once

#define MANGLE_FUNCTION(name) JOIN1(FUNC_PREFIX, JOIN2(_, name))
#define MANGLE_STRUCT(name) JOIN1(STRUCT_PREFIX, name)
#define JOIN1(a, name) JOIN2(a, name)
#define JOIN2(a, b) a ## b
