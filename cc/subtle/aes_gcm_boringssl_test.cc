// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "tink/subtle/aes_gcm_boringssl.h"

#include <string>
#include <vector>

#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "tink/util/test_util.h"
#include "gtest/gtest.h"

namespace crypto {
namespace tink {
namespace subtle {
namespace {

// Some test vectors generated by Wycheproof.
struct TestVector {
  std::string message;
  std::string key;
  std::string nonce;
  std::string aad;
  std::string ciphertext;
};
static const std::vector<TestVector> test_vector(
    {{"001d0c231287c1182784554ca3a21908", "5b9604fe14eadba931b0ccf34843dab9",
      "028318abc1824029138141a2", "",
      "26073cc1d851beff176384dc9896d5ff0a3ea7a5487cb5f7d70fb6c58d038554"},
     {"001d0c231287c1182784554ca3a21908", "5b9604fe14eadba931b0ccf34843dab9",
      "921d2507fa8007b7bd067d34", "00112233445566778899aabbccddeeff",
      "49d8b9783e911913d87094d1f63cc7651e348ba07cca2cf04c618cb4d43a5b92"},
     {"2035af313d1346ab00154fea78322105", "aa023d0478dcb2b2312498293d9a9129",
      "0432bc49ac34412081288127", "aac39231129872a2",
      "eea945f3d0f98cc0fbab472a0cf24e874bb9b4812519dadf9e1232016d068133"},
     {"41", "279db37ca5bfe9e65909abc914155b94", "2d6cffe7e604862f9ea40844", "",
      "76e5adeb571aee5c269e048f210045c942"},
     {"38576942693d4762", "79708b4a8b3eefba013cb33079efafbe",
      "7f4d73eee38d3cd39675bda2", "",
      "ebf0d695d68810f743bc43e8ab3b58b54ed7c29f08ce310f"},
     {"3a4b43345767484b634d7659615e4f73", "9d8a5c24bdc435912af2d1ee302035b4",
      "35ce70b390c811fcb3aeae46", "",
      "28a4f2e23f0a67a6d840d695bab2ba56e2722ec14498618d9cf54492d18dc89d"},
     {"5b69773a3857405e4265457a7446534a635e4c715e3b3f3f",
      "b1bb6f6fb5e7d4952dd77047e3eb6588", "06131d6837da496bc289feff", "",
      "df1aff6c813cb50aa40ebd44162dd3d14a1f2f43ec52ac56d84041e74a295618eeb4d5f2"
      "3dcd2079"},
     {"35773a6261473c55375b79654864574c", "a3d5ee5b36312a804fce9bef5283e145",
      "1dfb1b18be4d2bc2ddcda65b", "b9",
      "ddce596ee5c6d89cf5875ce4cef317e66e1baf768ba06482c383ec109a531cc5"},
     {"65507866526d414d504b454b6d52546c", "8ab05f2658b88a39dbca0c8318e4851b",
      "90e7dff1c6dda261447f7bab", "a4a0f20e09153034ee4724081d447f",
      "76b519149f968c50b4a009d373acb99dd8dcd6d002785e6bae2599ec63684936"},
     {"5b494d45553a423e7667623653423044", "56f86c1d52ca91de377309021746366b",
      "611342cf4477e48102be4136", "4591b9cb34859b18d66181f44922656c",
      "89a55935dfed3adf777e61d93cd0c6af0341b3034c8bb17d10940ab1dbbb8ae2"},
     {"39646b3a5f3a6f6f6742363342376349", "3071055b76738f7564cbbfb8e22a1dc3",
      "1de645fe206091c5e531b3df",
      "ae4c6df58a95a7d991e3127258368c165a7f5ab6a9d05a403e5e1f3ab59c22a0",
      "daca96f8370cc4fc24878bff5662f464ba6d0d0779ec2484457adcf51b2e4ac9"},
     {"ebd4a3e10cf6d41c50aeae007563b072", "00112233445566778899aabbccddeeff",
      "000000000000000000000000", "",
      "f62d84d649e56bc8cfedc5d74a51e2f7ffffffffffffffffffffffffffffffff"},
     {"d593c4d8224f1b100c35e4f6c4006543", "00112233445566778899aabbccddeeff",
      "ffffffffffffffffffffffff", "",
      "431f31e6840931fd95f94bf88296ff6900000000000000000000000000000000"},
     {"7fd49ba712d0d28f02ef54ed18db43f8", "00112233445566778899aabbccddeeff",
      "00112233445566778899aabb", "",
      "d8eba6a5a03403851abc27f6e15d84c000000000000000000000000000000000"},
     {"00010203040506070809",
      "92ace3e348cd821092cd921aa3546374299ab46209691bc28b8752d17f123c20",
      "00112233445566778899aabb", "00000000ffffffff",
      "e27abdd2d2a53d2f136b9a4a2579529301bcfb71c78d4060f52c"},
     {"", "29d3a44f8723dc640239100c365423a312934ac80239212ac3df3421a2098123",
      "00112233445566778899aabb", "aabbccddeeff",
      "2a7d77fa526b8250cb296078926b5020"},
     {"44", "f26b6a407f64135e7bdb9e89b262033546d64468411b7ec85b107757c9399614",
      "0bf364508963617860e248bf", "", "96c7898d70c21d7fc010d7de78b2e4e724"},
     {"343838484c463d50",
      "74c1425fb40a2b7eb3e053c77a330bdc2adb3b5775bda363c332c0240bb59d6d",
      "b809b3333eda23738c91e4f6", "",
      "e856f37462ff1443bb20f525bc4950f3a72785aa3f99fb77"},
     {"44776b30536d6d54457938375868685a",
      "2de58132592a46f4a94fd3ee71879be4ae77c062ad4fa866c5f3998de01b08e8",
      "58f58703b8767f88e121819f", "",
      "ca1cf57cd2c700325bec17a5ffcf377a3b1fcd17e0485afcb591824f686d6d4e"},
     {"56564d315b414741454c5f3c656e70647838796b78686850",
      "9c066464e76404c77a99979699d339dcad9fe2457bc0cce9fffdb302e47abb7d",
      "f74bd054131fa7f21cdfdad3", "",
      "d044d8f6ca0d18c6428fd7b2d88d8ffba4a89adbfc99a10c9600853bed661db896537dda"
      "58e5cccf"},
     {"3b716446444c623e683550593b674e47",
      "778c34f97a5e38178c9462a10c5f015935a95f24ec78c9c2e4d8df48e2ab429e",
      "444e908a15764d8ba9d62a07", "fb",
      "dde934f84768832ab74af7075c76ace70d9012fe7176841c259981b46600ee91"},
     {"4b6e4c783d6c4e3766554143423b6d72",
      "1be10f9c0be2b8f3ef930cfb90bd811c97c1bdb054b04725aff19020ac256128",
      "cbb57a02ab9984c1ae6dd835", "f3ab7a843f1c5a09ef23710b604b7b",
      "b02a58413603e2bf2530ec7602dadd3065ec4fb1e816bf8518cce2103639c3c7"},
     {"4a5e7960653c7836635e664930646a44",
      "02c70cb2422d1e8197b12ff62b7d4b2c3d788c11da20d0bfc9e9e66f5645d79f",
      "1fc396c9514394088fc44cd1", "d45fabdd6a41c0bf097a3eba02b6fc10",
      "74ac1a781516f11d69c14b344ed179676bb5eb2605648ddae460b5d30cda4283"},
     {"575777544230404363656f6d3a4b403a",
      "87aa120c5042694a563bfcd989215d3e77f36a9a7bd84c68678d88c9295a307b",
      "dd5b8d8ab86fd4024e1da335",
      "cb4bb68dfa0630a6a6181b734666794ef4a440a32785868ba970c2a31e6101ff",
      "51491471ab18447e13615868a180db8aea7bdc6a22e1920a541b1db4f9ac2441"},
     {"6e", "12a7ef1343a5f243b8cb004328934142a14c33b37b1a5ed6",
      "e83988b330933f02ae268a61", "", "37928f1f43e433192e949e064929e85301"},
     {"4076426236615d4b", "d802953840b9548a46cf694f5758bbd6d8cfa99761394b28",
      "a94427326f0bbf9f414ceee2", "",
      "d9d29357445f5c59b9f65ce05c877afe9bbc406e63fb035b"},
     {"623a68687442506f6554794d64417474",
      "d7134b9568c9652b2c6811e67b43d8a293810ae5bc1b2473",
      "3f283003948366639ef3fbe0", "",
      "dbc0574b4477849cda3d7309ffd5cb24906bd6afe24c2e2b2b29018948566e6f"},
     {"3f4f787461776e39674b684d3b4e517150326a5831326361",
      "4eb3b20e5c1cbb2179cab49a40488d55291cac38673d8ab1",
      "10369c2a423bf06356529eeb", "",
      "bbd488ec174956602052de5d52e2512dda1e76d2b94f8bbb4a2eaba7ee34cbb4cc2f8739"
      "dddd17f9"}});

TEST(AesGcmBoringSslTest, testBasic) {
  std::string key(test::HexDecodeOrDie("000102030405060708090a0b0c0d0e0f"));
  auto res = AesGcmBoringSsl::New(key);
  EXPECT_TRUE(res.ok()) << res.status();
  auto cipher = std::move(res.ValueOrDie());
  std::string message = "Some data to encrypt.";
  std::string aad = "Some data to authenticate.";
  auto ct = cipher->Encrypt(message, aad);
  EXPECT_TRUE(ct.ok()) << ct.status();
  EXPECT_EQ(ct.ValueOrDie().size(), message.size() + 12 + 16);
  auto pt = cipher->Decrypt(ct.ValueOrDie(), aad);
  EXPECT_TRUE(pt.ok()) << pt.status();
  EXPECT_EQ(pt.ValueOrDie(), message);
}

TEST(AesGcmBoringSslTest, testVectors) {
  for (const TestVector& test : test_vector) {
    std::string key = test::HexDecodeOrDie(test.key);
    std::string nonce = test::HexDecodeOrDie(test.nonce);
    std::string ct = test::HexDecodeOrDie(test.ciphertext);
    std::string aad = test::HexDecodeOrDie(test.aad);
    auto cipher = std::move(AesGcmBoringSsl::New(key).ValueOrDie());
    auto decrypted = cipher->Decrypt(nonce + ct, aad);
    EXPECT_TRUE(decrypted.ok()) << decrypted.status();
    if (decrypted.ok()) {
      EXPECT_EQ(test.message, test::HexEncode(decrypted.ValueOrDie()));
    }
  }
}

TEST(AesGcmBoringSslTest, testModification) {
  std::string key(test::HexDecodeOrDie("000102030405060708090a0b0c0d0e0f"));
  auto cipher = std::move(AesGcmBoringSsl::New(key).ValueOrDie());
  std::string message = "Some data to encrypt.";
  std::string aad = "Some data to authenticate.";
  std::string ct = cipher->Encrypt(message, aad).ValueOrDie();
  EXPECT_TRUE(cipher->Decrypt(ct, aad).ok());
  // Modify the ciphertext
  for (size_t i = 0; i < ct.size() * 8; i++) {
    std::string modified_ct = ct;
    modified_ct[i / 8] ^= 1 << (i % 8);
    EXPECT_FALSE(cipher->Decrypt(modified_ct, aad).ok()) << i;
  }
  // Modify the additional data
  for (size_t i = 0; i < aad.size() * 8; i++) {
    std::string modified_aad = aad;
    modified_aad[i / 8] ^= 1 << (i % 8);
    auto decrypted = cipher->Decrypt(ct, modified_aad);
    EXPECT_FALSE(decrypted.ok()) << i << " pt:" << decrypted.ValueOrDie();
  }
  // Truncate the ciphertext
  for (size_t i = 0; i < ct.size(); i++) {
    std::string truncated_ct(ct, 0, i);
    EXPECT_FALSE(cipher->Decrypt(truncated_ct, aad).ok()) << i;
  }
}

TEST(AesGcmBoringSslTest, testAadEmptyVersusNullStringView) {
  const std::string key(test::HexDecodeOrDie("000102030405060708090a0b0c0d0e0f"));
  auto cipher = std::move(AesGcmBoringSsl::New(key).ValueOrDie());
  const std::string message = "Some data to encrypt.";
  const absl::string_view aad;
  const std::string ct = cipher->Encrypt(message, aad).ValueOrDie();
  EXPECT_TRUE(cipher->Decrypt(ct, aad).ok());
}

}  // namespace
}  // namespace subtle
}  // namespace tink
}  // namespace crypto

int main(int ac, char* av[]) {
  testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
