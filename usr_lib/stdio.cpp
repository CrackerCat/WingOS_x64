#include <klib/kernel_util.h>
#include <klib/string_util.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char temp_buf[64];

int vsn_printf_out(bool just_print, char *buffer, uint64_t count, const char *data)
{
    if (just_print == true)
    {
        sys::write_console(data, count);
        return 1;
    }
    else if (buffer != nullptr)
    {
        strncpy(buffer, data, count);
        buffer = buffer + count;
        return 1;
    }
    return 0;
}

int vsn_printf(bool just_print, char *buffer /* nullptr say no buffer just print it*/, uint64_t count, const char *format, va_list argument)
{
    size_t written = 0;
    while (*format != '\0')
    {
        size_t maxrem = 0xffffffffff - written;

        if (format[0] != '%' || format[1] == '%')
        {
            if (format[0] == '%')
                format++;
            size_t amount = 1;
            while (format[amount] && format[amount] != '%')
                amount++;
            if (maxrem < amount)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!vsn_printf_out(just_print, buffer, amount, format))
                return written;
            format += amount;
            written += amount;
            continue;
        }

        const char *format_begun_at = format++;

        if (*format == 'c')
        {
            format++;
            char c = (char)va_arg(argument, int /* char promotes to int */);
            if (!maxrem)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!vsn_printf_out(just_print, buffer, sizeof(c), &c))
                return 0;
            written++;
        }
        else if (*format == 's')
        {
            format++;
            const char *str = va_arg(argument, const char *);
            size_t len = strlen(str);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!vsn_printf_out(just_print, buffer, len, str))
                return -1;
            written += len;
        }
        else if (*format == 'x')
        {
            format++;
            uint64_t d = va_arg(argument, uint64_t);
            if (d == 0)
            {
                char c = '0';
                vsn_printf_out(just_print, buffer, sizeof(c), &c);
                written += 1;
            }
            else
            {
                for (int i = 0; i < 64; i++)
                {
                    temp_buf[i] = 0;
                }
                sys::int_to_string<uint64_t>(temp_buf, 'x', d);
                size_t len = strlen(temp_buf);
                if (maxrem < len)
                {
                    // TODO: Set errno to EOVERFLOW.
                    return -1;
                }
                if (!vsn_printf_out(just_print, buffer, len, temp_buf))
                    return -1;
                written += len;
            }
        }
        else
        {
            format = format_begun_at;
            size_t len = strlen(format);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!vsn_printf_out(just_print, buffer, len, format))
                return -1;
            written += len;
            format += len;
        }
    }
    return written;
}

int sprintf(char *buffer, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    int return_value = vsn_printf(false, buffer, 0, format, va);
    va_end((va));
    return return_value;
}

int printf(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    int return_value = vsn_printf(true, nullptr, 0, format, va);
    va_end((va));
    return return_value;
}

int vsprintf(char *buffer, const char *format, va_list vlist)
{
    int return_value = vsn_printf(false, buffer, 0, format, vlist);
    return return_value;
}

FILE *fopen(const char *pathname, const char *mode)
{
    FILE *f = (FILE *)malloc(sizeof(FILE));
    f->file_element = sys::file(); // mode are not supported for the moment :^(
    f->file_element.open(pathname);
    return f;
}
int fclose(FILE *stream)
{
    stream->file_element.close();
    free(stream);
    return 0;
}

int fseek(FILE *stream, long offset, int whence)
{
    if (whence == SEEK_SET)
    {
        stream->file_element.seek(offset);
    }
    else if (whence == SEEK_CUR)
    {
        stream->file_element.seek(stream->file_element.get_cursor_pos() + offset);
    }
    return 1;
}
long ftell(FILE *stream)
{
    return stream->file_element.get_cursor_pos();
}

size_t fread(void *ptr, size_t size, size_t count, FILE *stream)
{
    return stream->file_element.read((uint8_t *)ptr, size * count);
}

int fgetc(FILE *stream)
{
    if (stream->file_element.get_file_length() == stream->file_element.get_cursor_pos() + 1)
    {
        return EOF;
    }
    else
    {

        uint8_t *data = (uint8_t *)malloc(1);
        stream->file_element.read(data, 1);
        uint8_t d = *data;
        free(data);
        return d;
    }
}

int ungetc(int c, FILE *stream)
{
    return 0;
}

int feof(FILE *stream)
{
    if (fgetc(stream) == EOF)
    {
        return true;
    }
    return false;
}

int ferror(FILE *stream)
{
    return true; // error not supported for the moment
}
