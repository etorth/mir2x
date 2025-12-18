#include <type_traits>
#include <utility>

#include "aesf.hpp"
#include "strf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"

#define ECB 0
#define CTR 0
#define CBC 1
#include "aes.hpp"

static_assert(AES_KEYLEN == 16);
static_assert(AES_BLOCKLEN == 16);

#define AES_ctx_ptr(p) std::launder(reinterpret_cast<AES_ctx *>(p))

aesf::AES::AES(const char *key, uint64_t iv1, uint64_t iv2)
{
    static_assert(AES_storage_alignment >= alignof(AES_ctx));
    static_assert(AES_storage_size      >=  sizeof(AES_ctx));

    const auto keysv = to_sv(str_haschar(key) ? key : "0299dc69a0a0f4de");
    uint8_t keybuf[AES_KEYLEN];

    for(size_t done = 0; done < AES_KEYLEN;){
        const size_t copied = std::min<size_t>(AES_KEYLEN - done, keysv.size());
        std::memcpy(keybuf + done, keysv.data(), copied);
        done += copied;
    }

    std::construct_at<AES_ctx>(reinterpret_cast<AES_ctx *>(storage));

    uint8_t ivbuf[16];
    std::memcpy(ivbuf    , &iv1, 8);
    std::memcpy(ivbuf + 8, &iv2, 8);

    AES_init_ctx_iv(AES_ctx_ptr(storage), keybuf, ivbuf);
}

void aesf::AES::encrypt(void *data, size_t size)
{
    fflassert(data);
    fflassert(size);
    fflassert(size % 16 == 0, size);
    AES_CBC_encrypt_buffer(AES_ctx_ptr(storage), reinterpret_cast<uint8_t *>(data), size);
}

void aesf::AES::decrypt(void *data, size_t size)
{
    fflassert(data);
    fflassert(size);
    fflassert(size % 16 == 0, size);
    AES_CBC_decrypt_buffer(AES_ctx_ptr(storage), reinterpret_cast<uint8_t *>(data), size);
}
