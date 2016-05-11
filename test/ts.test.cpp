/*
 * ========================================================
 *
 *       Filename:  ts.test.cpp
 *
 *    Description:  test ts module
 *
 *        Version:  1.0
 *        Created:  12/16/2012 03:58:50 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  liuyang1 (liuy), liuyang1@mail.ustc.edu.cn
 *   Organization:  ustc
 *
 * ========================================================
 */

#include "gtest/gtest.h"
#include "mylog.h"
#include "ts.h"

using namespace m2ts;

TEST(CRC32,POSITIVE)
{
	unsigned char data[]={0x00,0xb0,0x0d,0x00,0x01,0xc1,0x00,0x00, 0x00,0x01,0xef,0xff,0x36,0x90,0xe2,0x3d};
	EXPECT_EQ(chkCRC32(data,sizeof(data)),true);

	unsigned char data1[]={0x02,0xb0,0x12,0x00,0x01,0xc1,0x00,0x00,0xe1,0x00,0xf0,0x00,0x1b,0xe1,0x00,0xf0,0x00,0x15,0xbd,0x4d,0x56};
	EXPECT_EQ(chkCRC32(data1,sizeof(data1)),true);
}
TEST(DataPin,Postive)
{
	unsigned char a=0x01,b=0x2b;
	unsigned int ret=(a<<8)|b;
	EXPECT_EQ(ret,0x12b);
}

TEST(ParseTSHeader,Postive)
{
	class ParseTSHeader p;
	m2ts::ErrorCode ret;

	// test 1
	unsigned char data[]={0x47,0x40,0x00,0x10};
	ret = p.run(data);
	EXPECT_EQ(p.m_TShf->sync_byte                   , 0x47);
	EXPECT_EQ(p.m_TShf->transport_priority          , 0);
	EXPECT_EQ(p.m_TShf->payload_unit_start_indicator, 1);
	EXPECT_EQ(p.m_TShf->pid_h                       , 0);
	EXPECT_EQ(p.m_TShf->pid_l                       , 0);
	EXPECT_EQ(p.m_TShf->transport_error_indicator   , 0);
	EXPECT_EQ(p.m_TShf->continuity_counter          , 0);
	EXPECT_EQ(p.m_TShf->adaptation_field_control    , 1);
	EXPECT_EQ(p.m_TShf->transport_scrambling_control, 0);
	// test ret
	EXPECT_EQ(p.oPID,0);
	EXPECT_EQ(ret,OK);

	// test2
	unsigned char data1[]={0x47,0x4f,0xff,0x10};
	ret=p.run(data1);
	EXPECT_EQ(p.m_TShf->sync_byte                   , 0x47);
	EXPECT_EQ(p.m_TShf->transport_priority          , 0);
	EXPECT_EQ(p.m_TShf->payload_unit_start_indicator, 1);
	EXPECT_EQ(p.m_TShf->pid_h                       , 0xf);
	EXPECT_EQ(p.m_TShf->pid_l                       , 0xff);
	EXPECT_EQ(p.m_TShf->transport_error_indicator   , 0);
	EXPECT_EQ(p.m_TShf->continuity_counter          , 0);
	EXPECT_EQ(p.m_TShf->adaptation_field_control    , 1);
	EXPECT_EQ(p.m_TShf->transport_scrambling_control, 0);
	// test ret
	EXPECT_EQ(p.oPID                                , 4095);// 0xfff==4095
	EXPECT_EQ(ret                                   , OK);

	// test3
	unsigned char data2[]={0x47,0x41,0x00,0x30};
	ret=p.run(data2);
	EXPECT_EQ(p.m_TShf->sync_byte                   , 0x47);
	EXPECT_EQ(p.m_TShf->transport_priority          , 0);
	EXPECT_EQ(p.m_TShf->payload_unit_start_indicator, 1);
	EXPECT_EQ(p.m_TShf->pid_h                       , 0x1);
	EXPECT_EQ(p.m_TShf->pid_l                       , 0x00);
	EXPECT_EQ(p.m_TShf->transport_error_indicator   , 0);
	EXPECT_EQ(p.m_TShf->continuity_counter          , 0);
	EXPECT_EQ(p.m_TShf->adaptation_field_control    , 3);
	EXPECT_EQ(p.m_TShf->transport_scrambling_control, 0);
	// test ret
	EXPECT_EQ(p.oPID                                , 256);// 0x100==256
	EXPECT_EQ(ret                                   , OK);

	// test4 PES data,no start
	uchar data3[]={0x47,0x01,0x00,0x12,0x6b,0x69,0x70};
	ret = p.run(data3);
	EXPECT_EQ(p.m_TShf->sync_byte                   , 0x47);
	EXPECT_EQ(p.m_TShf->transport_priority          , 0);
	EXPECT_EQ(p.m_TShf->payload_unit_start_indicator, 0);// not start
	EXPECT_EQ(p.m_TShf->pid_h                       , 0x1);
	EXPECT_EQ(p.m_TShf->pid_l                       , 0x00);
	EXPECT_EQ(p.m_TShf->transport_error_indicator   , 0);
	EXPECT_EQ(p.m_TShf->continuity_counter          , 2);// not continuity_counter
	EXPECT_EQ(p.m_TShf->adaptation_field_control    , 1);
	EXPECT_EQ(p.m_TShf->transport_scrambling_control, 0);
	// test ret
	EXPECT_EQ(p.oPID                                , 256);// 0x100==256
	EXPECT_EQ(ret                                   , OK);
	
}

TEST(ParseAdapt,POSTIVE)
{
	class ParseTSHeader p;
	m2ts::ErrorCode ret;

	unsigned char data[TS_PACKET_SIZE]={0x47,0x41,0x00,0x10,0x00};
	ret = p.run(data);
	EXPECT_EQ(p.m_TSAf,(TS_Adaptation_Field_fix*)NULL);
	EXPECT_EQ(p.oAdaptLen,0);
	EXPECT_EQ(p.oDataPtr,data+4);
	EXPECT_EQ(p.oDataLen,TS_PACKET_SIZE-4);
	EXPECT_EQ(ret,OK);

	unsigned char data1[TS_PACKET_SIZE]={0x47,0x41,0x00,0x30,
		0x07,0x10,
		0xff,0xff,0xfa,0x22,0xfe,0x00};
	ret = p.run(data1);
	EXPECT_EQ(p.m_TShf->adaptation_field_control,3);
	EXPECT_EQ(p.m_TSAf,(TS_Adaptation_Field_fix*)(data1+1+4));
	EXPECT_EQ(p.oAdaptLen,7);
	EXPECT_EQ(p.oDataPtr,data1+4+7+1);
	EXPECT_EQ(p.oDataLen,TS_PACKET_SIZE-4-7-1);
	EXPECT_EQ(ret,OK);
}

TEST(ParsePAT,POSTIVE)
{
	class ParsePAT p;
	ErrorCode ret;

	unsigned char data[TS_PACKET_SIZE]={0x00,0xb0,0x0d,0x00,0x01,0xc1,0x00,0x00,
	0x00,0x01,0xef,0xff,
	0x36,0x90,0xe2,0x3d};
	ret = p.run(data);
	EXPECT_EQ(p.m_PATh.table_id,0);
	uint len=getPATSectionLength(p.m_PATh);
	EXPECT_EQ(len,13);
	uint id = getPATTransportStreamID(p.m_PATh);
	EXPECT_EQ(id,1);
	EXPECT_EQ(p.m_PATh.current_next_indicator,1);
	EXPECT_EQ(p.m_PATh.section_number,0);
	EXPECT_EQ(p.m_PATh.last_section_number,0);
	EXPECT_EQ(p.m_PATlstlen,1);
	if(p.m_PATlstlen>=1){
		EXPECT_EQ(ntohs(p.m_PATlst[0].program_number),1);
		EXPECT_EQ(getProgramPID(p.m_PATlst[0]),0xfff);
	}
	EXPECT_EQ(ret,OK);
}

TEST(ParsePMT,POSTIVE)
{
	class ParsePMT p;
	ErrorCode ret;

	unsigned char data[TS_PACKET_SIZE]={0x02,0xb0,0x12,0x00,0x01,0xc1,0x00,0x00,0xe1,0x00,0xf0,0x00,
		0x1b,0xe1,0x00,0xf0,0x00,
		0x15,0xbd,0x4d,0x56};
	ret = p.run(data);

	EXPECT_EQ(p.m_PMTh.table_id,2);
	uint len=getPMTSectionLength(p.m_PMTh);
	EXPECT_EQ(len,18);
	uint program_num = getPMTProgramNum(p.m_PMTh);
	EXPECT_EQ(program_num,1);
	EXPECT_EQ(p.m_PMTh.section_number,0);
	EXPECT_EQ(p.m_PMTh.last_section_number,0);
	uint pcrpid = getPMTPCRPID(p.m_PMTh);
	EXPECT_EQ(pcrpid,0x100);
	EXPECT_EQ(getPMTProgramInfoLength(p.m_PMTh),0);

	EXPECT_EQ(p.m_PMTlstlen,1);

	EXPECT_EQ(p.m_PMTlst[0].stream_type,0x1b);
	EXPECT_EQ(p.m_PMTlst[0].elementary_pid_h,0x01);
	EXPECT_EQ(p.m_PMTlst[0].elementary_pid_l,0x00);
	EXPECT_EQ(getElementaryPID(p.m_PMTlst[0]),256);
	EXPECT_EQ(p.m_PMTlst[0].es_info_length_h,0x00);
	EXPECT_EQ(p.m_PMTlst[0].es_info_length_l,0x00);
	EXPECT_EQ(getESInfoLength(p.m_PMTlst[0]),0);

	EXPECT_EQ(ret,OK);
	
}

TEST(ParsePES,POSTIVE)
{
	class ParsePES p;
	ErrorCode ret;

	unsigned char data[]={0x00,0x00,0x01,
			0xe0,0x2f,0xfe,
			0x80,0xc0,0x0a,
			0x31,0x00,0x01,0x00,0x01,
			0x1f,0xff,0xff,0xe8,0x8b,
			0x00,0x00,0x00,0x01};
	ret = p.run(data,sizeof(data));

	EXPECT_EQ(p.m_PEShf.stream_id,0xe0);
	EXPECT_EQ(getPESPayloadLength(p.m_PEShf),12286);
	EXPECT_EQ(p.m_payloadlen,12273);

	EXPECT_EQ(p.m_PESho.pes_header_data_len,10);
	EXPECT_EQ(p.oPTS,0);
	EXPECT_EQ(p.oDTS,0x1fffff445);//?
	EXPECT_EQ(p.m_payloadreadlen,4);
	EXPECT_EQ(p.m_payload[0],0x00);
	EXPECT_EQ(p.m_payload[1],0x00);
	EXPECT_EQ(p.m_payload[2],0x00);
	EXPECT_EQ(p.m_payload[3],0x01);

	p.dump();
	EXPECT_EQ(ret,OK);
}
#if 0
TEST(ParseTS,POSTIVE)
{
	char filename[]="../../testobj/a.hd.ts";
	FILE* fp = fopen(filename,"rb");
	if(fp==NULL){
		printf("not found test file,quit");
		FAIL();
	}
#define N_TS_PACKET	4
	unsigned char data[TS_PACKET_SIZE*N_TS_PACKET];// only handle 4 ts packet
	int n = fread(data,sizeof(unsigned char),TS_PACKET_SIZE*N_TS_PACKET,fp);
	if(n!=TS_PACKET_SIZE*N_TS_PACKET){
		printf("read data failed");
		FAIL();
	}

	ParseM2TS p;
	ErrorCode ret;
	for(unsigned int i=1;i<N_TS_PACKET;i++){
		ret = p.pushData(data+i*TS_PACKET_SIZE,TS_PACKET_SIZE);
		EXPECT_EQ(ret,OK);
	}
	EXPECT_EQ(p.pidinfolen,1);

	PIDinfo *info = &p.pidinfolst[0];
	EXPECT_EQ(info->stream_type,0x1b);
	EXPECT_EQ(info->pid,0x100);
	EXPECT_EQ(info->packetCnt,1);

	ParsePES *pes = &(info->m_ParsePES);
	EXPECT_EQ(pes->m_PEShf.stream_id,0xe0);
	EXPECT_EQ(getPESPayloadLength(pes->m_PEShf),12286);
	EXPECT_EQ(pes->m_payloadlen,12273);
	EXPECT_EQ(pes->m_PESho.pes_header_data_len,10);
	EXPECT_EQ(pes->oPTS,0);
	EXPECT_EQ(pes->oDTS,0x1fffff445);
	
	uchar *ans=p.pidinfolst[0].m_ParsePES.m_payload;
	EXPECT_EQ(ans[0],0x00);
	EXPECT_EQ(ans[1],0x00);
	EXPECT_EQ(ans[2],0x00);
	EXPECT_EQ(ans[3],0x01);

	fclose(fp);
	SUCCEED();
}
TEST(HKTSFile,Positive)
{
	char filename[]="../../testobj/hk.ts";
	FILE* fp=fopen(filename,"rb");
	if(fp==NULL){
		printf("not found test file [%s],quit\n",filename);
		FAIL();
	}
	unsigned char data[TS_PACKET_SIZE];
	ParseM2TS p;
	ErrorCode ret;
	for(uint i=0;i<10;i++){
		int n =fread(data,sizeof(uchar),TS_PACKET_SIZE,fp);
		if(n!=TS_PACKET_SIZE){
			printf("read data failed,readed %d data\n",i*TS_PACKET_SIZE+n);
			FAIL();
		}
		ret = p.pushData(data,TS_PACKET_SIZE);
		if(i!=0)// skip first TS packet
			EXPECT_EQ(ret,OK);
	}
}
#endif
