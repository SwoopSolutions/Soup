# RSA

```cpp
#include <soup/rsa.hpp>
```

## Key Import

Soup supports key exchange as defined by PKCS#1, X.509, PEM and JWK. However, all exchange formats have quite a bit of overhead, especially considering that you only want a couple of integers.

### OpenSSL Export

The following PHP code can be used to extract the underlying integers from an OpenSSL private key object:

```php
$priv = openssl_pkey_get_private("file://priv.key");

echo "n = ".gmp_strval(gmp_import(openssl_pkey_get_details($priv)["rsa"]["n"]))."\n";
echo "e = ".gmp_strval(gmp_import(openssl_pkey_get_details($priv)["rsa"]["e"]))."\n";
echo "p = ".gmp_strval(gmp_import(openssl_pkey_get_details($priv)["rsa"]["p"]))."\n";
echo "q = ".gmp_strval(gmp_import(openssl_pkey_get_details($priv)["rsa"]["q"]))."\n";
```

### Public Key Import

If you wish to use the public key with Soup, you only need to know the modulus (`n`).

```cpp
using namespace soup::literals;

soup::RsaPublicKey pub{
    "8446817108090355031541999500037472701172098105390664609736912338763723234754560584168907115333937153377507172109710102394091436024688692795634373262383717"_b
};
```

Although it can't hurt to verify that `e` is, in fact, 65537 — if it isn't, that's one more parameter for the constructor.

### Private Key Import

You really only need to know the 2 prime numbers (`p` and `q`) to use an RSA private key.

```cpp
using namespace soup::literals;

auto priv = soup::RsaKeypair(
    "96529209707922958264660626622151327182265565708623147261613126577409795199887"_b,
    "87505296413890087200392682183900465764322220376584167643884573751015402662091"_b
).getPrivate();
```

Although this will perform a handful of large calculations to construct a `soup::RsaPrivateKey`. Instead, you may wish to call the constructor directly. All values needed can be read from Soup or OpenSSL. Note that what Soup calls `dp`, `dq`, `qinv`, OpenSSL calls `dmp1`, `dmq1`, `iqmp`, respectively.

## Sign & Verify

Signing and verifying messages is done with simple templated functions. The template parameter is the hashing algorithm, which can be any descendant of `soup::CryptoHashAlgo`.

```cpp
soup::Bigint signature_bigint = priv.sign<soup::sha1>("It's me, I promise!");
```

A common encapsulation for the signature is base64-encoded binary data, which you can easily convert it to with Soup:

```cpp
#include <soup/base64.hpp>

std::string signature_base64 = soup::base64::encode(signature_bigint.toMessage());
```

Finally, to decode and verify the signature:

```cpp
auto signature_bigint = soup::Bigint::fromBinary(soup::base64::decode(signature_base64));

if (pub.verify<soup::sha1>("It's me, I promise!", signature_bigint))
{
    // Success
}
else
{
    // Failure
}
```
