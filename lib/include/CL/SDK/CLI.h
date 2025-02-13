#pragma once

// OpenCL SDK includes
#include <CL/SDK/Options.h>

// STL includes
#include <stdlib.h> // realloc
#include <stdbool.h> // bool
#include <string.h> // memcpy, strcmp, strtoul

// cargs includes
#include <cargs.h>

typedef struct cag_option cag_option;

cag_option *add_CLI_options(cag_option *opts, size_t *const num_opts,
                            cag_option *add_opts, size_t add_num_opts)
{
    cag_option *tmp = NULL;

    tmp = (cag_option *)realloc(
        opts, sizeof(cag_option) * (*num_opts + add_num_opts));
    if (tmp)
    {
        memcpy(tmp + *num_opts, add_opts, sizeof(cag_option) * add_num_opts);
        *num_opts += add_num_opts;
    }

    return tmp;
}

enum ParseState
{
    ParsedOK,
    NotParsed,
    ParseError
};

typedef enum ParseState ParseState;

cag_option DiagnosticOptions[] = {
    { .identifier = 'h',
      .access_letters = "h",
      .access_name = "help",
      .description = "Show this help" },

    { .identifier = 'q',
      .access_letters = "q",
      .access_name = "quiet",
      .description = "Suppress standard output" },

    { .identifier = 'v',
      .access_letters = "v",
      .access_name = "verbose",
      .description = "Extra informational output" }
};

ParseState parse_DiagnosticOptions(const char identifier,
                                   struct cl_sdk_options_Diagnostic *diag_opts)
{
    switch (identifier)
    {
        case 'q': diag_opts->quiet = true; return ParsedOK;
        case 'v': diag_opts->verbose = true; return ParsedOK;
    }
    return NotParsed;
}

cag_option SingleDeviceOptions[] = {
    { .identifier = 'p',
      .access_letters = "p",
      .access_name = "platform",
      .value_name = "(positive integer)",
      .description = "Index of platform to use" },

    { .identifier = 'd',
      .access_letters = "d",
      .access_name = "device",
      .value_name = "(positive integer)",
      .description = "Index of device to use" },

    { .identifier = 't',
      .access_letters = "t",
      .access_name = "type",
      .value_name = "(all|cpu|gpu|acc|def|cus)",
      .description = "Type of device to use" }
};

// TODO: error handling
cl_device_type get_dev_type(const char *in)
{
    if (!strcmp(in, "all"))
        return CL_DEVICE_TYPE_ALL;
    else if (!strcmp(in, "cpu"))
        return CL_DEVICE_TYPE_CPU;
    else if (!strcmp(in, "gpu"))
        return CL_DEVICE_TYPE_GPU;
    else if (!strcmp(in, "acc"))
        return CL_DEVICE_TYPE_ACCELERATOR;
    else if (!strcmp(in, "def"))
        return CL_DEVICE_TYPE_DEFAULT;
    else if (!strcmp(in, "cus"))
        return CL_DEVICE_TYPE_CUSTOM;
    else
        return CL_DEVICE_TYPE_ALL; // CL_INVALID_DEVICE_TYPE;// "Unkown device
                                   // type after cli parse. Should not have
                                   // happened."
}

ParseState
parse_SingleDeviceOptions(const char identifier,
                          cag_option_context *cag_context,
                          struct cl_sdk_options_SingleDevice *dev_opts)
{
    const char *value;

#define IF_ERR(op)                                                             \
    if ((value = cag_option_get_value(cag_context)))                           \
    {                                                                          \
        op;                                                                    \
        return ParsedOK;                                                       \
    }                                                                          \
    else                                                                       \
        return ParseError;

    switch (identifier)
    {
        case 'p': IF_ERR(dev_opts->triplet.plat_index = strtoul(value, NULL, 0))
        case 'd': IF_ERR(dev_opts->triplet.dev_index = strtoul(value, NULL, 0))
        case 't': IF_ERR(dev_opts->triplet.dev_type = get_dev_type(value))
    }
    return NotParsed;

#undef IF_ERR
}

#define PARS_OPTIONS(parser, state)                                            \
    do                                                                         \
    {                                                                          \
        if (state == NotParsed) state = parser;                                \
        if (state == ParseError)                                               \
        {                                                                      \
            fprintf(stderr, "Parse error\n");                                  \
            identifier = 'h';                                                  \
            state = ParsedOK;                                                  \
        }                                                                      \
    } while (0)
