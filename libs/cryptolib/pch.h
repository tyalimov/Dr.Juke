#pragma once

#include "framework.h"
#include "cryptolib.h"

#pragma warning (push, 0)
#   include <modes.h>    // CBC mode for AES
#   include <aes.h>      // AES algorithm
#   include <filters.h>
#   include <files.h>    // FileSource
#   include <eccrypto.h> // Elliptic curves
#pragma warning (pop)

#include <string>

