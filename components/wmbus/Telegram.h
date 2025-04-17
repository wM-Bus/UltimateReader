#pragma once
#include <vector>
#include <string>
#include "types.h"
#include <set>
#include <map>
#include <string>
#include "utils.h"
#include"address.h"

using namespace wmbusmeters;

struct AboutTelegram
{
    // wmbus device used to receive this telegram.
    std::string device;
    // The device's opinion of the rssi, best effort conversion into the dbm scale.
    // -100 dbm = 0.1 pico Watt to -20 dbm = 10 micro W
    // Measurements smaller than -100 and larger than -10 are unlikely.
    int rssi_dbm{};
    // WMBus or MBus
    FrameType type{};
    // time the telegram was received
    time_t timestamp;

    AboutTelegram(std::string dv, int rs, FrameType t, time_t ts = 0) : device(dv), rssi_dbm(rs), type(t), timestamp(ts) {}
    AboutTelegram() {}
};

struct MeterKeys
{
    std::vector<unsigned char> confidentiality_key;
   std::vector<unsigned char> authentication_key;

    bool hasConfidentialityKey() { return confidentiality_key.size() > 0; }
    bool hasAuthenticationKey() { return authentication_key.size() > 0; }
};


struct Explanation
{
    int pos{};
    int len{};
    std::string info;
    KindOfData kind{};
    Understanding understanding{};

    Explanation(int p, int l, const std::string& i, KindOfData k, Understanding u) :
        pos(p), len(l), info(i), kind(k), understanding(u) {}
};

enum class OutputFormat
{
    NONE, PLAIN, TERMINAL, JSON, HTML
};


struct Telegram
{
private:
    Telegram(Telegram&t) { }

public:
    Telegram() = default;

    AboutTelegram about;

    // If set to true then this telegram should be trigger updates.
    bool discard {};

    // If a warning is printed mark this.
    bool triggered_warning {};

    // The different addresses found,
    // the first is the dll_id_mvt, ell_id_mvt, nwl_id_mvt, and the last is the tpl_id_mvt.
    std::vector<Address> addresses;

    // If decryption failed, set this to true, to prevent further processing.
    bool decryption_failed {};

    // DLL
    int dll_len {}; // The length of the telegram, 1 byte.
    int dll_c {};   // 1 byte control code, SND_NR=0x44

    unsigned char dll_mfct_b[2]; //  2 bytes
    int dll_mfct {};

    unsigned char mbus_primary_address; // Single byte address 0-250 for mbus devices.
    unsigned char mbus_ci; // MBus control information field.

    std::vector<unsigned char> dll_a; // A field 6 bytes
    // The 6 a field bytes are composed of 4 id bytes, version and type.
    unsigned char dll_id_b[4] {};    // 4 bytes, address in BCD = 8 decimal 00000000...99999999 digits.
    std::vector<unsigned char> dll_id; // 4 bytes, human readable order.
    unsigned char dll_version {}; // 1 byte
    unsigned char dll_type {}; // 1 byte

    // ELL
    unsigned char ell_ci {}; // 1 byte
    unsigned char ell_cc {}; // 1 byte
    unsigned char ell_acc {}; // 1 byte
    unsigned char ell_sn_b[4] {}; // 4 bytes
    int   ell_sn {}; // 4 bytes
    unsigned char ell_sn_session {}; // 4 bits
    int   ell_sn_time {}; // 25 bits
    unsigned char ell_sn_sec {}; // 3 bits
    ELLSecurityMode ell_sec_mode {}; // Based on 3 bits from above.
    unsigned char ell_pl_crc_b[2] {}; // 2 bytes
    uint16_t ell_pl_crc {}; // 2 bytes

    unsigned char ell_mfct_b[2] {}; // 2 bytes;
    int   ell_mfct {};
    bool  ell_id_found {};
    unsigned char ell_id_b[6] {}; // 4 bytes;
    unsigned char ell_version {}; // 1 byte
    unsigned char ell_type {};  // 1 byte

    // NWL
    int nwl_ci {}; // 1 byte

    // AFL
    unsigned char afl_ci {}; // 1 byte
    unsigned char afl_len {}; // 1 byte
    unsigned char afl_fc_b[2] {}; // 2 byte fragmentation control
    uint16_t afl_fc {};
    unsigned char afl_mcl {}; // 1 byte message control

    bool afl_ki_found {};
    unsigned char afl_ki_b[2] {}; // 2 byte key information
    uint16_t afl_ki {};

    bool afl_counter_found {};
    unsigned char afl_counter_b[4] {}; // 4 bytes
    uint32_t afl_counter {};

    bool afl_mlen_found {};
    int afl_mlen {};

    bool must_check_mac {};
    std::vector<unsigned char> afl_mac_b;

    // TPL
    std::vector<unsigned char>::iterator tpl_start;
    int tpl_ci {}; // 1 byte
    int tpl_acc {}; // 1 byte
    int tpl_sts {}; // 1 byte
    int tpl_sts_offset {}; // Remember where the sts field is in the telegram, so
                           // that we can add more vendor specific decodings to it.
    int tpl_cfg {}; // 2 bytes
    TPLSecurityMode tpl_sec_mode {}; // Based on 5 bits extracted from cfg.
    int tpl_num_encr_blocks {};
    int tpl_cfg_ext {}; // 1 byte
    int tpl_kdf_selection {}; // 1 byte
    std::vector<unsigned char> tpl_generated_key; // 16 bytes
    std::vector<unsigned char> tpl_generated_mac_key; // 16 bytes

    bool  tpl_id_found {}; // If set to true, then tpl_id_b contains valid values.
    std::vector<unsigned char> tpl_a; // A field 6 bytes
    // The 6 a field bytes are composed of 4 id bytes, version and type.
    unsigned char tpl_id_b[4] {}; // 4 bytes
    unsigned char tpl_mfct_b[2] {}; // 2 bytes
    int   tpl_mfct {};
    unsigned char tpl_version {}; // 1 bytes
    unsigned char tpl_type {}; // 1 bytes

    // The format signature is used for compact frames.
    int format_signature {};

    std::vector<unsigned char> frame; // Content of frame, potentially decrypted.
    std::vector<unsigned char> parsed;  // Parsed bytes with explanations.
    int header_size {}; // Size of headers before the APL content.
    int suffix_size {}; // Size of suffix after the APL content. Usually empty, but can be MACs!
    int mfct_0f_index = -1; // -1 if not found, else index of the 0f byte, if found, inside the difvif data after the header.
    int mfct_1f_index = -1; // -1 if not found, else index of the 1f byte, if found, then there are more records in the next telegram.
    int force_mfct_index = -1; // Force all data after this offset to be mfct specific. Used for meters not using 0f.
    void extractFrame(std::vector<unsigned char> *fr); // Extract to full frame.
    void extractPayload(std::vector<unsigned char> *pl); // Extract frame data containing the measurements, after the header and not the suffix.
    void extractMfctData(std::vector<unsigned char> *pl); // Extract frame data after the DIF 0x0F.

    bool handled {}; // Set to true, when a meter has accepted the telegram.

    bool parseHeader(std::vector<unsigned char> &input_frame);
    bool parse(std::vector<unsigned char> &input_frame, MeterKeys *mk, bool warn);

    bool parseMBUSHeader(std::vector<unsigned char> &input_frame);
    bool parseMBUS(std::vector<unsigned char> &input_frame, MeterKeys *mk, bool warn);

    bool parseWMBUSHeader(std::vector<unsigned char> &input_frame);
    bool parseWMBUS(std::vector<unsigned char> &input_frame, MeterKeys *mk, bool warn);

    bool parseHANHeader(std::vector<unsigned char> &input_frame);
    bool parseHAN(std::vector<unsigned char> &input_frame, MeterKeys *mk, bool warn);

    void addAddressMfctFirst(const std::vector<unsigned char>::iterator &pos);
    void addAddressIdFirst(const std::vector<unsigned char>::iterator &pos);

    void print();

    // A vector of indentations and explanations, to be printed
    // below the raw data bytes to explain the telegram content.
    std::vector<Explanation> explanations;
    void addExplanationAndIncrementPos(std::vector<unsigned char>::iterator &pos, int len, KindOfData k, Understanding u, const char* fmt, ...);
    void setExplanation(std::vector<unsigned char>::iterator &pos, int len, KindOfData k, Understanding u, const char* fmt, ...);
    void addMoreExplanation(int pos, const char* fmt, ...);
    void addMoreExplanation(int pos, std::string json);

    // Add an explanation of data inside manufacturer specific data.
    void addSpecialExplanation(int offset, int len, KindOfData k, Understanding u, const char* fmt, ...);
    void explainParse(std::string intro, int from);
    std::string analyzeParse(OutputFormat o, int *content_length, int *understood_content_length);

    bool parserWarns() { return parser_warns_; }
    bool isSimulated() { return is_simulated_; }
    bool beingAnalyzed() { return being_analyzed_; }
    void markAsSimulated() { is_simulated_ = true; }
    void markAsBeingAnalyzed() { being_analyzed_ = true; }

    // The actual content of the (w)mbus telegram. The DifVif entries.
    // Mapped from their key for quick access to their offset and content.
    std::map<std::string,std::pair<int,DVEntry>> dv_entries;

    std::string autoDetectPossibleDrivers();

    // part of original telegram bytes, only filled if pre-processing modifies it
    std::vector<unsigned char> original;

private:

    bool is_simulated_ {};
    bool being_analyzed_ {};
    bool parser_warns_ = true;
    MeterKeys *meter_keys {};

    // Fixes quirks from non-compliant meters to make telegram compatible with the standard
    void preProcess();

    bool parseMBusDLLandTPL(std::vector<unsigned char>::iterator &pos);

    bool parseDLL(std::vector<unsigned char>::iterator &pos);
    bool parseELL(std::vector<unsigned char>::iterator &pos);
    bool parseNWL(std::vector<unsigned char>::iterator &pos);
    bool parseAFL(std::vector<unsigned char>::iterator &pos);
    bool parseTPL(std::vector<unsigned char>::iterator &pos);

    void printDLL();
    void printELL();
    void printNWL();
    void printAFL();
    void printTPL();

    bool parse_TPL_72(std::vector<unsigned char>::iterator &pos);
    bool parse_TPL_78(std::vector<unsigned char>::iterator &pos);
    bool parse_TPL_79(std::vector<unsigned char>::iterator &pos);
    bool parse_TPL_7A(std::vector<unsigned char>::iterator &pos);
    bool alreadyDecryptedCBC(std::vector<unsigned char>::iterator &pos);
    bool potentiallyDecrypt(std::vector<unsigned char>::iterator &pos);
    bool parseTPLConfig(std::vector<unsigned char>::iterator &pos);
    static std::string toStringFromELLSN(int sn);
    static std::string toStringFromTPLConfig(int cfg);
    static std::string toStringFromAFLFC(int fc);
    static std::string toStringFromAFLMC(int mc);

    bool parseShortTPL(std::vector<unsigned char>::iterator &pos);
    bool parseLongTPL(std::vector<unsigned char>::iterator &pos);
    bool checkMAC(std::vector<unsigned char> &frame,
                  std::vector<unsigned char>::iterator from,
                  std::vector<unsigned char>::iterator to,
                  std::vector<unsigned char> &mac,
                  std::vector<unsigned char> &mackey);
    bool findFormatBytesFromKnownMeterSignatures(std::vector<unsigned char> *format_bytes);
};

std::string manufacturer(int m_field);
std::string mediaType(int a_field_device_type, int m_field);
std::string mediaTypeJSON(int a_field_device_type, int m_field);
bool isCiFieldOfType(int ci_field, CI_TYPE type);
int ciFieldLength(int ci_field);
bool isCiFieldManufacturerSpecific(int ci_field);
std::string ciType(int ci_field);
std::string cType(int c_field);
bool isValidWMBusCField(int c_field);
bool isValidMBusCField(int c_field);
std::string ccType(int cc_field);

std::string vifKey(int vif); // E.g. temperature energy power mass_flow volume_flow
std::string vifUnit(int vif); // E.g. m3 c kwh kw MJ MJh

bool isCloseEnough(int media1, int media2);
LinkModeInfo* getLinkModeInfo(LinkMode lm);
LinkModeInfo* getLinkModeInfoFromBit(int bit);
std::string tostringFromELLSN(int sn);
std::string tostringFromTPLConfig(int cfg);
std::string tostringFromAFLFC(int fc);
std::string tostringFromAFLMC(int mc);
bool decrypt_ELL_AES_CTR(Telegram* t, std::vector<unsigned char>& frame, std::vector<unsigned char>::iterator& pos, std::vector<unsigned char>& aeskey);
bool decrypt_TPL_AES_CBC_IV(Telegram* t, std::vector<unsigned char>& frame, std::vector<unsigned char>::iterator& pos, std::vector<unsigned char>& aeskey,
    int* num_encrypted_bytes,
    int* num_not_encrypted_at_end);
bool decrypt_TPL_AES_CBC_NO_IV(Telegram* t, std::vector<unsigned char>& frame, std::vector<unsigned char>::iterator& pos, std::vector<unsigned char>& aeskey,
    int* num_encrypted_bytes,
    int* num_not_encrypted_at_end);
void incrementIV(unsigned char* iv, size_t len);
std::string frameTypeKamstrupC1(int ft);
void AES_CMAC(unsigned char* key, unsigned char* input, int length, unsigned char* mac);
void xorit(unsigned char* srca, unsigned char* srcb, unsigned char* dest, int len);
void shiftLeft(unsigned char* srca, unsigned char* srcb, int len);
// Decode only the standard defined bits in the tpl status byte. Ignore the top 3 bits.
// Return "OK" if sts == 0
std::string decodeTPLStatusByteOnlyStandardBits(unsigned char sts);
