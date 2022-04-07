/**
 * Transfering file using ICMP protocol - ISA project 2021, VUT FIT
 *
 * @file File loading and saving module
 * @author Andrej Hýroš (xhyros00)
 */


#ifndef ISA_FILE_HANDLER_H
#define ISA_FILE_HANDLER_H
typedef unsigned char BYTE;


/**
 *Check if file exists in current directory
 *
 * @param name name of to-be-checked file
 * @return true if file exists, false if otherwise
 */
bool exists(const std::string& name);

/**
 * Load file to vector of bytes
 * external source: https://stackoverflow.com/questions/15138353/how-to-read-a-binary-file-into-a-vector-of-unsigned-chars
 * @param filename name of to-be-loaded file
 */
std::vector<BYTE> load(const std::string &filename);

/**
 * Sace file from vector of bytes to current directory
 * external source: https://stackoverflow.com/questions/15138353/how-to-read-a-binary-file-into-a-vector-of-unsigned-chars
 * @param filename name of to-be-loaded file
 */
void save(const std::string &filename, const std::vector<BYTE>& vec);

/**
 * Save file from byte array
 * @param filename name of to-be-loaded file
 */
void arr_save(const std::string &filename, char *buffer, unsigned long size);

/**
 * Load file to byte array
 *
 * @param filename name of to-be-loaded file
 * @param filesize size of file in bytes
 * @return pointer to byte array loaded with file data
 */
BYTE * arr_load(const std::string &filename, int * filesize);


#endif //ISA_FILE_HANDLER_H
