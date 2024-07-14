#pragma once


#define STRINGIZE_IMPL(p) #p
#define STRINGIZE(p) STRINGIZE_IMPL(p)

#define CONCAT_IMPL(p1, p2) p1 ## p2
#define CONCAT(p1, p2) CONCAT_IMPL(p1, p2)

#define MAKE_UNIQUE_VAR(name) CONCAT(name, __COUNTER__)

#define CURRENT_LINE STRINGIZE(__LINE__)
#define CURRENT_FILE __FILE__

