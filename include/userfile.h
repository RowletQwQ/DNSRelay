// 读取本地文件，更新数据库中IP地址
#ifndef _USERFILE_H_
#define _USERFILE_H_
struct domin_table_data;

/**
 * @brief 初始化读取文件
 * 
 * @param file_name 文件名
 */
void init_read_file(const char *file_name);

/**
 * @brief 读取本地文件，更新数据库中IP地址
 * 
 * @param buffer 用来存放读取到的数据
 * @param len buffer的长度
 * @return int 读取到的数据条数
 */
int read_data(struct domin_table_data *buffer, int len);

/**
 * @brief 关闭文件
 * 
 */
void close_file();

/**
 * @brief 初始化写入文件
 * 
 * @param file_name 
 */
void init_write_file(const char *file_name);


/**
 * @brief 将数据写入文件
 * 
 * @param data 数据
 * @param len 数据长度
 */
void write_data(struct domin_table_data *data, int len);

#endif