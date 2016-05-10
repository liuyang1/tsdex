/*
 * ========================================================
 *
 *       Filename:  ts.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  12/12/2012 02:14:44 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liuyang1 (liuy), liuyang1@mail.ustc.edu.cn
 *   Organization:  ustc
 *
 * ========================================================
 */

#include <stdlib.h>
#include <unistd.h>
#include "ts.h"
#include "mylog.h"

using namespace m2ts;

m2ts::ParseM2TS g_tsparser;
#define _DUMP_INFO_ 1

/* ----------------------->> TSHeader <<--------------------------- */
void ParseTSHeader::clear() {
    m_TShf = NULL;
    oPID = -1;

    m_TSAf = NULL;
    oAdaptLen = 0;
    bzero((void *)&oPCR, sizeof(PCR_Field));
    bzero((void *)&oOPCR, sizeof(PCR_Field));

    oDataPtr = NULL;
}

ErrorCode ParseTSHeader::run(uchar *ptr)
{
    if (ptr == NULL) {
        return NoData;
    }

    clear();

    uchar *org = ptr;

    m_TShf = (TS_Header_fix *)ptr;
    ptr += sizeof(TS_Header_fix);

    if (m_TShf->sync_byte != SYNCCODE || m_TShf->transport_error_indicator == 1 ||
        m_TShf->transport_scrambling_control != 0) {
        NOTICE("TS fix header wrong");
        return IncorrectData;
    }
    oPID = getPID(*m_TShf);

    if (m_TShf->adaptation_field_control & 0x02) {
        oAdaptLen = *ptr;
        ptr += 1;
        if (oAdaptLen == 0) {
            DEBUG("adapt len must >= 1: %d,skip parseAdapt", oAdaptLen);
        } else {
            parseAdapt(ptr);
            ptr += oAdaptLen;
        }
    }
    if (m_TShf->adaptation_field_control & 0x01) {
        oDataPtr = ptr;
        oDataLen = TS_PACKET_SIZE - (ptr - org);
    } else {
        NOTICE("this ts packet NOT contain data?!");
        oDataPtr = NULL;
        oDataLen = 0;
        return IncorrectData;
    }
    return OK;
}

ErrorCode ParseTSHeader::parseAdapt(uchar *ptr)
{
    m_TSAf = (TS_Adaptation_Field_fix *)ptr;
    ptr += sizeof(TS_Adaptation_Field_fix);
    if (m_TSAf->PCR_flag) {
        oPCR = *(PCR_Field *)ptr;
        ptr += sizeof(PCR_Field);
    }
    if (m_TSAf->OPCR_flag) {
        oOPCR = *(PCR_Field *)ptr;
        ptr += sizeof(PCR_Field);
    }
    if (m_TSAf->splicing_point_flag) {
        DEBUG("skip splicing_point");
        ptr += 1;      // skip 8 bit
    }
    if (m_TSAf->transport_private_data_flag) {
        DEBUG("skip transport_private_data");
        uchar len = *ptr;
        ptr += 1 + len;
    }
    if (m_TSAf->adaptation_field_extension_flag) {
        DEBUG("skip adaptation_field_extension");
        uchar len = *ptr;
        ptr += 1 + len;
    }
    return OK;
}

void ParseTSHeader::dump()
{
#if _DUMP_INFO_ == 1
    DUMP("-----------------------------");
    DUMP("TS_Header_fix...");
    DUMP("PID                          0x%x", getPID(*m_TShf));
    // DUMP("transport_priority           %d",m_TShf->transport_priority);
    DUMP("payload_unit_start_indicator %d", m_TShf->payload_unit_start_indicator);
    // DUMP("transport_error_indicator    %d",m_TShf->transport_error_indicator);
    DUMP("continuity_counter           %d", m_TShf->continuity_counter);
    DUMP("adaptation_field_control     0x%x", m_TShf->adaptation_field_control);
    // DUMP("transport_scrambling_control 0x%x",m_TShf->transport_scrambling_control);
    if (!(m_TShf->adaptation_field_control & 0x02)) {
        DUMP("-----------------------------");
        return;        // not contain adaptation_field
    }
    DUMP("TS_Adaptation_Field...");
    DUMP("TS_Adaptation_Field len 1+%d", oAdaptLen);
    if (oAdaptLen != 0) {
        if (m_TSAf->discontinuity_indicator) {
            DUMP("discontinuity_indicator	1");
        }
        if (m_TSAf->PCR_flag) {
            DUMP("PCR						0x%x", oPCR);
        }
        if (m_TSAf->OPCR_flag) {
            DUMP("OPCR						0x%x", oOPCR);
        }
    }
    DUMP("-----------------------------");
#endif
}

/* ----------------------->> PAT <<------------------------------- */
void ParsePAT::clear()
{
    bzero(&m_PATh, sizeof(m_PATh));
    if (m_PATlst != NULL) {
        free(m_PATlst);
        m_PATlst = NULL;
    }
    m_PATlstlen = 0;
}

ErrorCode ParsePAT::run(uchar *ptr)
{
    clear();

    uchar *org = ptr;

    m_PATh = *(PAT_Header *)ptr;
    ptr += sizeof(PAT_Header);

    // check data default value
    if (m_PATh.table_id != TID_PAT) {// PAT table_id == 0
        return IncorrectType;
    }
    if (m_PATh.section_syntax_indicator != 0x01 || (m_PATh.reserved1 & 0x04) != 0x00) {
        NOTICE("PAT header wrong");
        return IncorrectData;
    }
    if (m_PATh.current_next_indicator != 1) {
        return NoVaildData;        // 无效数据
    }
    // 校验CRC值
    uint patlen = getPATSectionLength(m_PATh) + PAT_HEADER_LEN_BEFORE_IDC;
    if (chkCRC32(org, patlen) != true) {
        NOTICE("PAT CRC error");
        return IncorrectData;
    }

    uint section_length = getPATSectionLength(m_PATh);
    m_PATlstlen = section_length - PAT_HEADER_LEN_AFTER_IDC - CRC_32_SIZE;   // section_length -
                                                                             // header -CRC32
    if (m_PATlstlen % sizeof(PAT_Element) != 0) {
        NOTICE("PAT not contain integer PAT_Element");
        return IncorrectData;
    }
    m_PATlstlen /= sizeof(PAT_Element);

    if (m_PATlstlen != 0) {
        m_PATlst = (PAT_Element *)malloc(sizeof(PAT_Element) * m_PATlstlen);
        if (m_PATlst == NULL) {
            return NoMemory;
        }

        memcpy(m_PATlst, ptr, m_PATlstlen * sizeof(PAT_Element));
    }
    return OK;
}

void ParsePAT::dump() {
#if _DUMP_INFO_ == 1
    DUMP("-----------------------------");
    DUMP("PAT...");
    DUMP("table_id    %d", m_PATh.table_id);
    DUMP("stream_id   %d", getPATTransportStreamID(m_PATh));
    DUMP("PATlstlen   %d", m_PATlstlen);
    if (m_PATlstlen >= 1) {
        DUMP("PAT element 1...");
        DUMP("program_num %d", m_PATlst[0].program_number);
        DUMP("program_PID 0x%x", getProgramPID(m_PATlst[0]));
    }
    DUMP("-----------------------------");
#endif
}

/* ----------------------->> PMT <<------------------------------- */
void ParsePMT::clear()
{
    bzero(&m_PMTh, sizeof(m_PMTh));
    if (m_PMTlst != NULL) {
        free(m_PMTlst);
        m_PMTlst = NULL;
    }
    m_PMTlstlen = 0;
}

ErrorCode ParsePMT::run(uchar *ptr)
{
    clear();
    uchar *org = ptr;

    m_PMTh = *(PMT_Header *)ptr;
    ptr += sizeof(PMT_Header);

    if (m_PMTh.table_id != TID_PMT) {// PMT table_id == 2
        return IncorrectType;
    }
    if (m_PMTh.section_syntax_indicator != 0x01) {
        NOTICE("PMT section_syntax_indicator wrong");
        return IncorrectData;
    }
    if (m_PMTh.current_next_indicator != 1) {
        return NoVaildData;
    }
    if (getPMTPCRPID(m_PMTh) == PID_NUL) {
        WARN("PCRPID is PID_NUL,so NO exist PCR info");
    }
    // 检查CRC
    uint pmtlen = getPMTSectionLength(m_PMTh) + PMT_HEADER_LEN_BEFORE_IDC;
    if (chkCRC32(org, pmtlen) != true) {
        NOTICE("PMT CRC error");
        return IncorrectData;
    }

    uint section_length = getPMTSectionLength(m_PMTh);
    m_PMTlstlen = section_length - PMT_HEADER_LEN_AFTER_IDC - CRC_32_SIZE;    // section_length -
                                                                              // header - CRC
    if (m_PMTlstlen % sizeof(PMT_Element) != 0) {
        NOTICE("PMT not contain integer PMT_Element");
        return IncorrectData;
    }
    m_PMTlstlen /= sizeof(PMT_Element);

    m_PMTlst = (PMT_Element *)malloc(sizeof(PMT_Element) * m_PMTlstlen);
    if (m_PMTlst == NULL) {
        return NoMemory;
    }

    memcpy(m_PMTlst, ptr, m_PMTlstlen * sizeof(PMT_Element));

    return OK;
}

void ParsePMT::dump()
{
#if _DUMP_INFO_ == 1
    DUMP("-----------------------------");
    DUMP("PMT");
    DUMP("program_number     %d", getPMTProgramNum(m_PMTh));
    DUMP("PCR_PID            0x%x", getPMTPCRPID(m_PMTh));
    PMT_Element *t;
    for (uint i = 0; i < m_PMTlstlen; i++) {
        t = m_PMTlst + i;
        DUMP("%d: type %02x %-24s ele_pid %04x", i + 1, t->stream_type,
             StreamType2str(t->stream_type), getElementaryPID(*t));
    }
    DUMP("-----------------------------");
#endif
}

/* ----------------------->> PES <<------------------------------- */
void ParsePES::clear()
{
    bzero(&m_PEShf, sizeof(m_PEShf));
    bzero(&m_PESho, sizeof(m_PESho));
    m_payloadreadlen = 0;
    m_payloadlen = 0;
}

ErrorCode ParsePES::pushPayloadData(uchar *ptr, uint len)
{
    // INFO("all space %u bytes,already write %u data,preprare to write %u
    // bytes",m_payloadlen,m_payloadreadlen,len);
    if (m_payloadreadlen + len > m_payloadlen) {
        WARN("all %u ,already %u data,to write %u bytes, SO %u bytes discard", m_payloadlen,
             m_payloadreadlen, len, len - (m_payloadlen - m_payloadreadlen));
        memcpy(m_payload + m_payloadreadlen, ptr, m_payloadlen - m_payloadreadlen);  // only write
                                                                                     // partly data
        m_payloadreadlen = m_payloadlen;
        // 检验输出
        dumpBin(m_payload, m_payloadlen);
    } else {
        memcpy(m_payload + m_payloadreadlen, ptr, len);
        m_payloadreadlen += len;
    }
    // write to file
    if (m_payloadlen == m_payloadreadlen) {
        writePayloadFile();
    }

    return OK;
}

ErrorCode ParsePES::writePayloadFile()
{
    INFO("one pes payload %6u bytes", m_payloadlen);
#if 0
    if (!h264::isPrefix(m_payload)) {
        WARN("pes payload is NOT h264 nalu!");
    }
    // dump to echo nalu one file
    h264::dumpNalu((char *)m_payload, m_payloadlen);
    // dumo to only one file
    // h264::dumpOneFile((char*)m_payload,m_payloadlen);
#endif
    return OK;
}

ErrorCode ParsePES::run(uchar *ptr, uint len)
{
    if (m_payloadlen == PES_PAYLOAD_MAX_LENGTH) { // if no limit data, should dump last data to nalu
        m_payloadlen = m_payloadreadlen;
        writePayloadFile();
    }
    clear();

    uchar *org = ptr;

    m_PEShf = *(PES_Header_fix *)ptr;
    ptr += sizeof(PES_Header_fix);

    // TODO:添加验证性的过程
    if (len > TS_PACKET_SIZE - TS_HEADER_LEN) {
        NOTICE("too much data in ONE ts packet");
        return IncorrectData;
    }
    if (m_PEShf.start_prefix[0] != 0x00 || m_PEShf.start_prefix[1] != 0x00 ||
        m_PEShf.start_prefix[2] != 0x01) {
        NOTICE("pes packet start prefix WRONG");
        return IncorrectData;
    }

    if (hasPESHeaderOpt(m_PEShf.stream_id)) {
        parsePESHeaderOpt(ptr);
        ptr += sizeof(PES_Header_opt);
        ptr += m_PESho.pes_header_data_len;
    }

    if (m_payloadlen != 0 && !isFullData()) {
        NOTICE("last PES data no full,discard it! m_payloadlen %d m_payloadreadlen %d",
               m_payloadlen, m_payloadreadlen);
    }

    uint pesheadlen = ptr - org;
    m_payloadlen = getPESPayloadLength(m_PEShf);
    if (m_payloadlen == 0) {
        m_payloadlen = PES_PAYLOAD_MAX_LENGTH;
        NOTICE("pes payload length is NO LIMIT,set to %d bytes", m_payloadlen);
    } else {
        // 注意:整个PES包长度为PESPayloadLength+sizeof(PES_Header_fix)
        // 这里已经解析的长度为sizeof(PES_Header_fix)+len(PES_Header_opt)=pesheadlen
        // 数据部分长度为整个PES包的长度-已经解析部分长度
        // =(PESPayloadLength+sizeof(PES_Header_fix))-pesheadlen
        // =PESPayloadLength-pesheadlen+sizeof(PES_Header_fix)
        // =PESPayloadLength-(pesheadlen-sizeof(PES_Header_fix))
        m_payloadlen -= pesheadlen - sizeof(PES_Header_fix);
        if (m_payloadlen > PES_PAYLOAD_MAX_LENGTH) {
            ERROR("too much data,please set bigger PES_PAYLOAD_MAX_LENGTH");
        }
    }
    m_payloadreadlen = 0;
    if (m_payload == NULL) {
        return NoMemory;
    }
    if (org + len <= ptr) {
        NOTICE("really? not enough data to parse");
        NOTICE("all %d parse %d data,%d payload", len, ptr - org, len - (ptr - org));
        return IncorrectData;
    }
    return pushPayloadData(ptr, len - (ptr - org));
}

bool ParsePES::hasPESHeaderOpt(uchar stream_id)
{
    return stream_id != STREAMTYPE_STREAM_MAP
           && stream_id != STREAMTYPE_PADDING
           && stream_id != STREAMTYPE_PRIVATE_2
           && stream_id != STREAMTYPE_ECM
           && stream_id != STREAMTYPE_EMM
           && stream_id != STREAMTYPE_PS_DIR;
}

ErrorCode ParsePES::parsePESHeaderOpt(uchar *ptr)
{
    m_PESho = *(PES_Header_opt *)ptr;
    ptr += sizeof(PES_Header_opt);

    if (m_PESho.pts_dts_flag == 0x02) {
        oPTS = getVal(*(PTS_DTS_Header *)ptr);
        ptr += sizeof(PTS_DTS_Header);
    }
    if (m_PESho.pts_dts_flag == 0x03) {
        oPTS = getVal(*(PTS_DTS_Header *)ptr);
        ptr += sizeof(PTS_DTS_Header);
        oDTS = getVal(*(PTS_DTS_Header *)ptr);
        ptr += sizeof(PTS_DTS_Header);
    }
    if (m_PESho.escr_flag) {
        oESCR = getESCR(*(ESCR_Header *)ptr);
        oESCRext = getESCRext(*(ESCR_Header *)ptr);
        ptr += sizeof(ESCR_Header);
    }
    if (m_PESho.es_rate_flag) {
        oES_Rate = getESrate(*(ES_Rate_Header *)ptr);
        ptr += sizeof(ES_Rate_Header);
    }
    if (m_PESho.dsm_trick_mode_flag) {
        NOTICE("dsm_trick_mode NoImpl");
        ptr += TRICK_MODE_LEN;
    }
    if (m_PESho.additional_copy_info_flag) {
        NOTICE("PES additional_copy_info NoImpl");
    }
    return OK;

}

void ParsePES::dump()
{
#if _DUMP_INFO_ == 1
    DUMP("-----------------------------");
    DUMP("PES...");
    DUMP("stream_id         0x%x", m_PEShf.stream_id);
    DUMP("pes_packet_length %d", getPESPayloadLength(m_PEShf));
    DUMP("m_payloadlen      %d", m_payloadlen);
    if (getPESPayloadLength(m_PEShf) == 0) {
        DUMP("PES payload length is NO LIMIT");
    }
    if (hasPESHeaderOpt(m_PEShf.stream_id)) {
        if (m_PESho.pts_dts_flag == 0x02) {
            DUMP("PTS 0x%010llx time: %9.4fs", oPTS, PTSDTS2Time(oPTS));
        }
        if (m_PESho.pts_dts_flag == 0x03) {
            DUMP("PTS 0x%010llx time: %9.4fs", oPTS, PTSDTS2Time(oPTS));
            DUMP("DTS 0x%010llx time: %9.4fs", oDTS, PTSDTS2Time(oDTS));
        }
        if (m_PESho.escr_flag) {
            DUMP("ESCR %d ESCRext %d", oESCR, oESCRext);
        }
        if (m_PESho.es_rate_flag) {
            DUMP("ES_Rate %d", oES_Rate);
        }
    }
    DUMP("-----------------------------");
#endif
}

void ParsePES::dumpPayload()
{
#if _DUMP_INFO_ == 1
    DUMP("------------PES Data---------");
    dumpBin(m_payload, m_payloadreadlen);
    DUMP("-----------------------------");
#endif
}

/* ----------------------->> M2TS <<------------------------------- */
void ParseM2TS::clear() {
    if (pidinfolst != NULL) {
        delete[] pidinfolst;
        pidinfolst = NULL;
    }
    if (PMTPID != NULL) {
        delete[] PMTPID;
        PMTPID = NULL;
    }
    PMTNum = 0;
    pidinfolen = 0;
}

ErrorCode ParseM2TS::pushData(uchar *ptr, uint len)
{
    if (len != TS_PACKET_SIZE) {
        WARN("data size wrong!");
        return IncorrectData;
    }
    return callParseTSHeader(ptr);
}

bool isContinuity(uint oldcnt, uint newcnt) {
    return (oldcnt == 0xf && newcnt == 0) || (oldcnt + 1 == newcnt);
}

ErrorCode ParseM2TS::callParseTSHeader(uchar *ptr)
{
    uchar *org = ptr;
#if _DUMP_INFO_ == 1
    DUMP("dump origin ts data");
    dumpBin(ptr, TS_PACKET_SIZE);
#endif
    ParseTSHeader p;
    ErrorCode ret = p.run(ptr);
    if (ret != OK) {
        return ret;
    }
    p.dump();

    ptr = p.oDataPtr;  // to skip adaptation_field and other
    DEBUG("ts header %d bytes,payload %d bytes", ptr - org, TS_PACKET_SIZE - (ptr - org));
    DEBUG("PID 0x%x", p.oPID);
    if (p.oPID == PID_NUL) {
        return OK;
    }
    if (p.oPID == PID_PAT) {
        uchar ptr_field = *ptr;
        ptr += ptr_field + POINTER_FIELD_LEN;
        return callParsePAT(ptr);
    }
    for (unsigned int i = 0; i < PMTNum; i++) {
        ERROR("test %d %d", i, PMTPID[i]);
        if (p.oPID == PMTPID[i]) {
            ERROR("find!");
            uchar ptr_field = *ptr;
            ptr += ptr_field + POINTER_FIELD_LEN;
            return callParsePMT(ptr);
        }
    }
    for (unsigned int i = 0; i < pidinfolen; i++) {
        if (p.oPID == pidinfolst[i].pid) {
            pidinfolst[i].packetCnt++;
            // 连续性的标识符号处理
            if (pidinfolst[i].continuity_counter == COUNTER_UNINIT
                || (p.m_TSAf != NULL && p.m_TSAf->discontinuity_indicator)) {
                pidinfolst[i].continuity_counter = p.m_TShf->continuity_counter;
            } else {
                if (!isContinuity(pidinfolst[i].continuity_counter, p.m_TShf->continuity_counter)) {
                    WARN("PID %x Continuity error last %d new %d", pidinfolst[i].pid,
                         pidinfolst[i].continuity_counter, p.m_TShf->continuity_counter);
                    // 不要丢失数据
                    // return IncorrectData;
                }
                pidinfolst[i].continuity_counter = p.m_TShf->continuity_counter;
            }
            // 解析PES头或者压入数据
            if (p.m_TShf->payload_unit_start_indicator == 1) {
                if (TS_PACKET_SIZE - (ptr - org) == 0) {
                    NOTICE("skip one null pes packet");
                    return ret;
                }
                ErrorCode ret = pidinfolst[i].m_ParsePES.run(ptr, TS_PACKET_SIZE - (ptr - org));
                pidinfolst[i].m_ParsePES.dump();
                return ret;
            } else {
                return pidinfolst[i].m_ParsePES.pushPayloadData(ptr, TS_PACKET_SIZE - (ptr - org));
            }
            if (pidinfolst[i].m_ParsePES.isFullData()) {
                pidinfolst[i].m_ParsePES.dumpPayload();
            }
        }
    }
    WARN("PID 0x%x not get by PAT or PMT", p.oPID);
    if (p.m_TShf->payload_unit_start_indicator == 1) {
        ParsePES pPES;
        pPES.run(ptr, TS_PACKET_SIZE - p.oDataLen);
        pPES.dump();
    }
    return UnknownPID;
}

ErrorCode ParseM2TS::callParsePAT(uchar *ptr)
{
    ParsePAT p;
    ErrorCode ret = p.run(ptr);
    p.dump();
    if (ret != OK) {
        return ret;
    }

    if (p.m_PATlstlen == 0) {
        INFO("contain no PS");
        return OK;
    }
    PMTNum = p.m_PATlstlen;
    PMTPID = new unsigned int[PMTNum];
    for (unsigned int i = 0; i < PMTNum; i++) {
        PMTPID[i] = getProgramPID(p.m_PATlst[i]);
        NOTICE("%d PMT pid 0x%x", i, PMTPID[i]);
    }
    return OK;
}

ErrorCode ParseM2TS::callParsePMT(uchar *ptr)
{
    ParsePMT p;
    ErrorCode ret = p.run(ptr);
    p.dump();
    if (ret != OK) {
        return ret;
    }

    if (p.m_PMTlstlen == 0) {
        return OK;
    }
    pidinfolen = p.m_PMTlstlen;
    pidinfolst = new PIDinfo[pidinfolen];
    if (pidinfolst == NULL) {
        return NoMemory;
    }
    unsigned int i;
    for (i = 0; i < pidinfolen; i++) {
        pidinfolst[i].clear();
        pidinfolst[i].pid = getElementaryPID(p.m_PMTlst[i]);
        pidinfolst[i].stream_type = p.m_PMTlst[i].stream_type;
    }

    return OK;
}

/*
   int ParseM2TSex::loop(){
        unsigned char ptr[TS_PACKET_SIZE];
        int size;
        while(true){
                size=bufReadFix(in,ptr,TS_PACKET_SIZE);
                if(size==TS_PACKET_SIZE)
                {
                        parser.pushData(ptr,size);
                }
                // if(in->usedsize==0)
                        pthread_testcancel();
        }
        return OK;
   }  */

// CRC ----------------------------------------------------
// CRC32 lookup table for polynomial 0x04c11db7
// ref:http://forums.dvbowners.com/index.php?showtopic=10773
static uint crctab[256] = {
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
    0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
    0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
    0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
    0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
    0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
    0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
    0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
    0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
    0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
    0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
    0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
    0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
    0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
    0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
    0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
    0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
    0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
    0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
    0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
    0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
    0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

uint crc32(uchar *buf, uint len)
{
    uint crc = 0xffffffff;
    for (uint i = 0; i != len; i++) {
        crc = (crc << 8) ^ crctab[((crc >> 24) ^ buf[i]) & 0xff];
    }
    return crc;
}
