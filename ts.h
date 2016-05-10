/*
 * ========================================================
 *
 *       Filename:  ts.h
 *
 *    Description:  parse ts packet
 *
 *        Version:  1.0
 *        Created:  12/11/2012 02:36:09 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liuyang1 (liuy), liuyang1@mail.ustc.edu.cn
 *   Organization:  ustc
 *
 * ========================================================
 */

#ifndef _TS_H_
#define _TS_H_
#include "base.h"
#include "mylog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <endian.h>

#include <limits.h>
#if CHAR_BIT != 8// this contain in limits.h
#error "char bit must be 8!!"
#endif
#if USHRT_MAX != 65535
#error "short bit must be 16!!"
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
// #warning	"TS module only for little endian"
#elif __BYTE_ORDER == __BIG_ENDIAN
#error          "TS module only for little endian"
#endif

#ifndef NULL
#define NULL 0
#endif


#define TS_PACKET_SIZE  188
#define TS_HEADER_LEN   4


#define TRICK_MODE_LEN  1
#define ADDITIONAL_COPY_INFO_LEN        1
#define PES_CRC_LEN             2

#define POINTER_FIELD_LEN       1

#define CRC_32_SIZE             4

#define SYNCCODE                0x47

/*  pid definition */
#define PID_PAT                 0x0000
#define PID_CAT                 0x0001
#define PID_TSDT                0x0002

#define PID_NUL                 0x1fff

#define PID_UNINIT                      0xffff

#define COUNTER_UNINIT          0xff

/*  table id definition */
#define TID_PAT                 0x00
#define TID_CAT                 0x01
#define TID_PMT                 0x02

/*  stream type definition  */
#define STREAMTYPE_STREAM_MAP 0xbc
#define STREAMTYPE_PRIVATE_1  0xbd
#define STREAMTYPE_PADDING    0xbe
#define STREAMTYPE_PRIVATE_2  0xbf
/*  0b110x xxxx 13818 audio / 11172-3 audio  */
/*  0b1110 xxxx H.262 / 13818-2 / 11172-3 video */
#define STREAMTYPE_ECM          0xf0
#define STREAMTYPE_EMM          0xf1
#define STREAMTYPE_DSM_CC       0xf2
#define STREAMTYPE_13522        0xf3
/*  0b1111 xxxx reserved */
#define STREAMTYPE_PS_DIR       0xff

#define STREAMTYPE_RESERVED     0x00
#define STREAMTYPE_11172_V      0x01
#define STREAMTYPE_H262         0x02
#define STREAMTYPE_11172_A      0x03
#define STREAMTYPE_13813_3_A    0x04
#define STREAMTYPE_13818_1      0x05
#define STREAMTYPE_13818_1_PEC  0x06
#define STREAMTYPE_13522_MHEG   0x07
#define STREAMTYPE_13818_DSM_CC 0x08
#define STREAMTYPE_11722_1      0x09
#define STREAMTYPE_H264_AVC     0x1b
#define STREAMTYPE_H264_SVC     0x1f
#define case2str(x)   case x: return (# x);


#define MAX_PTS_DTS                             0x1ffffffffll
#define PTS_DTS_FREQ                    9 * 1000 * 1000

#define PES_PAYLOAD_MAX_LENGTH  (1024 * 1024)

namespace m2ts {
    inline double PTSDTS2Time(u64 pts) {
        return pts / (PTS_DTS_FREQ + 0.0);
    }

    inline const char *StreamType2str(ushort stream_type) {
        switch (stream_type) {
            case STREAMTYPE_H264_SVC:
                return "H.264 SVC stream";
            case STREAMTYPE_H264_AVC:
                return "H.264 AVC stream";
                case2str(STREAMTYPE_RESERVED);
                case2str(STREAMTYPE_11172_V);
                case2str(STREAMTYPE_H262);
                case2str(STREAMTYPE_11172_A);
                case2str(STREAMTYPE_13813_3_A);
                case2str(STREAMTYPE_13818_1);
                case2str(STREAMTYPE_13818_1_PEC);
                case2str(STREAMTYPE_13522_MHEG);
                case2str(STREAMTYPE_13818_DSM_CC);
                case2str(STREAMTYPE_11722_1);
            default:
                return "unknown stream type";
        }
    }

}

namespace m2ts
{
    typedef struct _TS_Header_fix {
        uchar sync_byte;
        uchar pid_h                        : 5;
        uchar transport_priority           : 1;
        uchar payload_unit_start_indicator : 1;
        uchar transport_error_indicator    : 1;
        uchar pid_l;
        uchar continuity_counter           : 4;
        uchar adaptation_field_control     : 2;
        uchar transport_scrambling_control : 2;
    } TS_Header_fix;

    inline ushort getPID(TS_Header_fix tsh)
    {
        return tsh.pid_h << 8 | tsh.pid_l;
    }

    typedef struct _TS_Adaptation_Field_fix {
        uchar adaptation_field_extension_flag      : 1;
        uchar transport_private_data_flag          : 1;
        uchar splicing_point_flag                  : 1;
        uchar OPCR_flag                            : 1;
        uchar PCR_flag                             : 1;
        uchar elementary_stream_priority_indicator : 1;
        uchar random_access_indicator              : 1;
        uchar discontinuity_indicator              : 1;
    } TS_Adaptation_Field_fix;


    typedef struct {
        uchar base_1;
        uchar base_2;
        uchar base_3;
        uchar base_4;
        // one byte
        uchar ext_h : 1;
        uchar reserved : 6;
        uchar base_5 : 1;
        //
        uchar ext_l;
    } PCR_Field;

    inline long long getPCRbase(PCR_Field pcr)
    {
        return pcr.base_5
               | (pcr.base_4 << 1)
               | (pcr.base_3 << 9)
               | (pcr.base_2 << 17)
               | (pcr.base_1 << 25);
    }

    inline long long getPCRext(PCR_Field pcr)
    {
        return (pcr.ext_h << 8) | (pcr.ext_l);
    }

    typedef struct {
        uchar table_id;
        //
        uchar section_length_h         : 4;
        uchar reserved1                : 2;
        uchar unused                   : 1;
        uchar section_syntax_indicator : 1;
        //
        uchar section_length_l;
        uchar transport_stream_id_h;
        uchar transport_stream_id_l;

        uchar current_next_indicator   : 1;
        uchar version_number           : 5;
        uchar reserved2                : 2;

        uchar section_number;
        uchar last_section_number;
    } PAT_Header;
#define PAT_HEADER_LEN_BEFORE_IDC       3
#define PAT_HEADER_LEN_AFTER_IDC        5

    inline ushort getPATSectionLength(PAT_Header pat) {
        return pat.section_length_h << 8 | pat.section_length_l;
    }

    inline ushort getPATTransportStreamID(PAT_Header pat) {
        return pat.transport_stream_id_h << 8 | pat.transport_stream_id_l;
    }

    typedef struct {
        ushort program_number;
        uchar  program_pid_h   : 5;
        uchar  reserved        : 3;
        uchar  program_pid_l;
    } PAT_Element;

    inline ushort getProgramPID(PAT_Element pat) {
        return pat.program_pid_h << 8 | pat.program_pid_l;
    }

    inline ushort getProgramNum(PAT_Element pat) {
        return ntohs(pat.program_number);
    }

    typedef struct {
        uchar  table_id;
        //
        uchar  section_length_h         : 4;
        uchar  reserved1                : 3;
        uchar  section_syntax_indicator : 1;
        uchar  section_length_l;
        uchar program_number_h;
        uchar program_number_l;
        //
        uchar  current_next_indicator   : 1;
        uchar  version_number           : 5;
        uchar  reserved2                : 2;
        uchar  section_number;
        uchar  last_section_number;
        //
        uchar  pcr_pid_h                : 5;
        uchar  reserved3                : 3;
        //
        uchar  pcr_pid_l;
        uchar  program_info_length_h    : 4;
        uchar  reserved4                : 4;
        //
        uchar  program_info_length_l;
    } PMT_Header;
#define PMT_HEADER_LEN_BEFORE_IDC       3
#define PMT_HEADER_LEN_AFTER_IDC        9


    inline ushort getPMTSectionLength(PMT_Header pmt) {
        return pmt.section_length_h << 8 | pmt.section_length_l;
    }

    inline ushort getPMTProgramNum(PMT_Header pmt) {
        return pmt.program_number_h << 8 | pmt.program_number_l;
    }

    inline ushort getPMTPCRPID(PMT_Header pmt) {
        return pmt.pcr_pid_h << 8 | pmt.pcr_pid_l;
    }

    inline ushort getPMTProgramInfoLength(PMT_Header pmt) {
        return pmt.program_info_length_h << 8 | pmt.program_info_length_l;
    }

    typedef struct {
        uchar stream_type;
        uchar elementary_pid_h  : 5;
        uchar reserved          : 3;
        uchar elementary_pid_l;
        uchar es_info_length_h  : 4;
        uchar reserved1         : 4;
        uchar es_info_length_l;
    } PMT_Element;   // 5 byte;

    inline ushort getElementaryPID(PMT_Element pmt) {
        return pmt.elementary_pid_h << 8 | pmt.elementary_pid_l;
    }

    inline ushort getESInfoLength(PMT_Element pmt) {
        return pmt.es_info_length_h << 8 | pmt.es_info_length_l;
    }

    typedef struct {
        uchar  start_prefix[3];
        uchar  stream_id;
        uchar pes_packet_length_h;
        uchar pes_packet_length_l;
    } PES_Header_fix;

    inline ushort getPESPayloadLength(PES_Header_fix peshf) {
        return peshf.pes_packet_length_h << 8 | peshf.pes_packet_length_l;
    }

    typedef struct {
        uchar original_or_copy          : 1;
        uchar copyright                 : 1;
        uchar data_alignment_indicator  : 1;
        uchar pes_priority              : 1;
        uchar pes_scrambling_control    : 2;
        uchar reserved                  : 2;
        //
        uchar pes_extension_flag        : 1;
        uchar pes_crc_flag              : 1;
        uchar additional_copy_info_flag : 1;
        uchar dsm_trick_mode_flag       : 1;
        uchar es_rate_flag              : 1;
        uchar escr_flag                 : 1;
        uchar pts_dts_flag              : 2;
        //
        uchar pes_header_data_len;
    } PES_Header_opt;

    typedef struct {
        uchar marker_bit1    : 1;
        uchar pts_dts_32_30  : 3;
        uchar reserved       : 4;
        uchar pts_dts_29_22;
        uchar marker_bit2    : 1;
        uchar pts_dts_21_15  : 7;
        uchar pts_dts_14_7;
        uchar marker_bit3    : 1;
        uchar pts_dts_6_0    : 7;
    } PTS_DTS_Header;

    inline unsigned long long getVal(PTS_DTS_Header pts) {
        u64 ret = (pts.pts_dts_32_30 << 15) | (pts.pts_dts_29_22 << 7) | (pts.pts_dts_21_15);
        ret <<= 15;
        ret |= (pts.pts_dts_14_7 << 7) | pts.pts_dts_6_0;
        return ret;
    }

    typedef struct {
        uchar escr_base_29_28  : 2;
        uchar marker_bit1      : 1;
        uchar escr_base_32_30  : 3;
        uchar reserved         : 2;
        uchar escr_base_27_20;
        uchar escr_base_14_13  : 2;
        uchar marker_bit2      : 1;
        uchar escr_base_19_15  : 5;
        uchar escr_base_12_5;
        uchar escr_ext_8_7     : 2;
        uchar marker_bit3      : 1;
        uchar escr_base_4_0    : 5;
        uchar marker_bit4      : 1;
        uchar escr_ext_6_0     : 7;
    } ESCR_Header;

    inline unsigned long long getESCR(ESCR_Header a) {
        return a.escr_base_4_0 | (a.escr_base_12_5 << 5) | (a.escr_base_14_13 << 13) |
               (a.escr_base_19_15 << 15) | (a.escr_base_27_20 << 20)
               | (a.escr_base_29_28 << 28) | (a.escr_base_32_30 << 30);
    }

    inline ushort getESCRext(ESCR_Header escr) {
        return escr.escr_ext_6_0 | (escr.escr_ext_8_7 << 7);
    }

    typedef struct {
        uchar es_rate_21_15 : 7;
        uchar marker_bit1   : 1;
        uchar es_rate_14_7;
        uchar marker_bit2   : 1;
        uchar es_rate_6_0   : 7;
    } ES_Rate_Header;

    inline uint getESrate(ES_Rate_Header esr) {
        return esr.es_rate_6_0 | (esr.es_rate_14_7 << 7) | (esr.es_rate_21_15 << 15);
    }

    // TODO
    // pes ext

}
namespace m2ts {
    enum ErrorCode {
        OK = 0,
        NoData,
        IncorrectData,
        IncorrectType,
        NoVaildData,
        TooMuchData = 5,
        UnknownPID,
        NoMemory,
    };
    static char ErrorMsg[][128] = {
        "OK",
        "NoData",
        "IncorrectData",
        "IncorrectType",
        "NoVaildData",
        "TooMuchData",
        "UnknownPID",
        "NoMemory",
    };
    inline void showErrorMsg(ErrorCode c) {
        if (c != OK) {
            INFO(ErrorMsg[c]);
        }
    }

}
// end of M2TS struct
namespace m2ts {
    class ParseTSHeader
    {
        public:
            TS_Header_fix *m_TShf;
            uint oPID;

            TS_Adaptation_Field_fix *m_TSAf;
            uint oAdaptLen;
            PCR_Field oPCR;
            PCR_Field oOPCR;

            uchar *oDataPtr;
            uint oDataLen;

            ParseTSHeader() {
            }
            ~ParseTSHeader() {
            }
            void clear();
            // 进行解析过程
            // in:	ts包数据
            // out: 解析结果,o****数据
            //		错误码
            ErrorCode run(uchar *ptr);
            // 解析adaptation_field_extension_flag
            // in:  adaptation_field数据
            // out: 解析结果,o****数据
            ErrorCode parseAdapt(uchar *ptr);
            void dump();
        private:
            DISALLOW_COPY_AND_ASSIGN(ParseTSHeader);
    };

    class ParsePAT {
        public:
            PAT_Header m_PATh;
            PAT_Element *m_PATlst;
            uint m_PATlstlen;

            ParsePAT()
                : m_PATlst(NULL) {
            }

            ~ParsePAT() {
                clear();
            }
            void clear();
            void dump();
            ErrorCode run(uchar *ptr);
        private:
            DISALLOW_COPY_AND_ASSIGN(ParsePAT);
    };
    class ParsePMT {
        public:
            PMT_Header m_PMTh;
            PMT_Element *m_PMTlst;
            uint m_PMTlstlen;

            ParsePMT()
                : m_PMTlst(NULL) {
            }
            ~ParsePMT() {
            }
            void clear();
            void dump();
            ErrorCode run(uchar *ptr);
        private:
            DISALLOW_COPY_AND_ASSIGN(ParsePMT);
    };
    class ParsePES {
        public:
            PES_Header_fix m_PEShf;
            PES_Header_opt m_PESho;

            unsigned long long oDTS;
            unsigned long long oPTS;
            unsigned long long oESCR;
            ushort oESCRext;
            ushort oES_Rate;

            uchar *m_payload;
            uint m_payloadreadlen;            // already read payload data len
            uint m_payloadlen;            // payload length

            ErrorCode initBuf() {
                m_payload = (uchar *)malloc(sizeof(uchar) * PES_PAYLOAD_MAX_LENGTH);
                if (m_payload == NULL) {
                    WARN("PES packet malloc NoMemory");
                    return NoMemory;
                }
                return OK;
            }

            void release() {
                if (m_payload != NULL) {
                    free(m_payload);
                }
            }

            ParsePES() : m_payload(NULL) {
                initBuf();
            }
            ~ParsePES() {
                clear(); release();
            }
            void clear();
            void dump();
            void dumpPayload();
            ErrorCode writePayloadFile();
            ErrorCode pushPayloadData(uchar *ptr, uint len);
            ErrorCode run(uchar *ptr, uint len);
            bool hasPESHeaderOpt(uchar stream_id);
            ErrorCode parsePESHeaderOpt(uchar *ptr);
            bool isFullData() {
                return m_payloadlen == m_payloadreadlen;
            }

        private:
            DISALLOW_COPY_AND_ASSIGN(ParsePES);
    };
    class PIDinfo {
        public:
            uchar stream_type;
            uint pid;
            uchar continuity_counter;
            ParsePES m_ParsePES;

            // stat info
            uint packetCnt;
            void clear()
            {
                stream_type = 0x00;
                pid = 0;
                continuity_counter = COUNTER_UNINIT;
                packetCnt = 0;
            }

            PIDinfo() {
            }
        private:
            DISALLOW_COPY_AND_ASSIGN(PIDinfo);
    };
    class ParseM2TS
    {
        public:
            uint *PMTPID;
            uint PMTNum;
            PIDinfo *pidinfolst;
            uint pidinfolen;

            ParseM2TS() : PMTPID(NULL), PMTNum(0), pidinfolst(NULL), pidinfolen(0) {
            }
            ~ParseM2TS() {
                clear();
            }
            void clear();
            ErrorCode pushData(uchar *ptr, uint len);
        private:
            ErrorCode callParseTSHeader(uchar *ptr);
            ErrorCode callParsePAT(uchar *ptr);
            ErrorCode callParsePMT(uchar *ptr);
            DISALLOW_COPY_AND_ASSIGN(ParseM2TS);
    };

    /*
       class ParseM2TSex
       {
            public:
                    ParseM2TS parser;
                    Buffer*		in;
                    Buffer*		out;
                    ParseM2TSex(Buffer* _in,Buffer* _out):in(_in),out(_out){}

                    int loop();
       }; */
}

extern m2ts::ParseM2TS g_tsparser;

//计算CRC
uint crc32(uchar *buf, uint len);

inline bool chkCRC32(uchar *buf, uint len) {
    int ret = crc32(buf, len);
#ifdef _DUMP_INFO_
    dumpBin(buf, len);
    DUMP("CRC: %X", ret);
#endif
    return ret == 0;
}

#endif// end of _TS_H_
