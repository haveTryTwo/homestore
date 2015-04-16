#ifndef HOME_STORE_LOG_H_
#define HOME_STORE_LOG_H_

namespace homestore
{

const int FILE_PATH_LEN        = 2048;
const int MAX_BUF_LEN          = 2048;
   
enum LOG_LVL
{
    LOG_LVL_DEBUG = 0,
    LOG_LVL_TRACE,
    LOG_LVL_INFO, 
    LOG_LVL_WARN,
    LOG_LVL_ERR,
    LOG_LVL_FATAL_ERR,
};

#define LOG_FATAL_ERR(format, ...) print_log(__FILE__, __LINE__, LOG_LVL_FATAL_ERR, format, ## __VA_ARGS__)
#define LOG_ERR(format, ...) print_log(__FILE__, __LINE__, LOG_LVL_ERR, format, ## __VA_ARGS__)
#define LOG_WARN(format, ...) print_log(__FILE__, __LINE__, LOG_LVL_WARN, format, ## __VA_ARGS__)
#define LOG_INFO(format, ...) print_log(__FILE__, __LINE__, LOG_LVL_INFO, format, ## __VA_ARGS__)
#define LOG_TRACE(format, ...) print_log(__FILE__, __LINE__, LOG_LVL_TRACE, format, ## __VA_ARGS__)
#define LOG_DEBUG(format, ...) print_log(__FILE__, __LINE__, LOG_LVL_DEBUG, format, ## __VA_ARGS__)

void set_log_level(int level);
void set_log_fmt(const char *log_path_fmt);
void log_init(const char *log_path_fmt, int level);
void log_stream_close();
void log_destroy();
void print_log(const char* file_path, int line_no, int log_level, const char* format, ...);

}

#endif
