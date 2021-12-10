#include "ima_adpcm_engine.h"
#include "byte_array.h"

#include <ctime>
#include <chrono>
#include <string>
#include <cassert>
#include <iostream>

/* ѡ�����ֵ */
#define IMAADPCM_MAX_VAL(a, b) (((a) > (b)) ? (a) : (b))

/* ѡ����Сֵ */
#define IMAADPCM_MIN_VAL(a, b) (((a) < (b)) ? (a) : (b))

/* ������min���ϲ�С��max */
#define IMAADPCM_INNER_VAL(val, min, max) \
  IMAADPCM_MAX_VAL(min, IMAADPCM_MIN_VAL(max, val))

#define HEADER_SIZE 48;

/* �����䶯�� */
static const int8_t index_table[16] = {
  -1, -1, -1, -1, 2, 4, 6, 8,
  -1, -1, -1, -1, 2, 4, 6, 8
};

/* ���������� */
static const uint16_t stepsize_table[89] = {
	  7,     8,     9,    10,    11,    12,    13,    14,
	 16,    17,    19,    21,    23,    25,    28,    31,
	 34,    37,    41,    45,    50,    55,    60,    66,
	 73,    80,    88,    97,   107,   118,   130,   143,
	157,   173,   190,   209,   230,   253,   279,   307,
	337,   371,   408,   449,   494,   544,   598,   658,
	724,   796,   876,   963,  1060,  1166,  1282,  1411,
   1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,
   3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,
   7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
  15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
  32767
};

struct RIFF		// 12
{
	std::array<uint8_t, 4> chunk_id = { 'R', 'I','F','F' };
	uint32_t chunk_size = 40;
	std::array<uint8_t, 4> format = { 'W', 'A','V','E' };
};

struct FMT		// 28
{
	std::array<uint8_t, 4> sub_chunk_id = { 'f', 'm','t',' ' };
	uint32_t sub_chunk_id_size = 20;
	uint16_t audio_format = static_cast<uint16_t>(Format::FORMAT_IMA_ADPCM);
	uint16_t num_channels = 1;
	uint32_t sample_rate = 12000;
	uint32_t byte_rate = sample_rate * 16;
	uint16_t block_align = 256;
	uint16_t bits_pre_sample = 4;
	uint16_t extra_param_size = 2;
	uint16_t sample_per_block = 505;
};

struct DATA		// 8
{
	std::array<uint8_t, 4> sub_chunk2_id = { 'd', 'a','t','a' };
	uint32_t sub_chunk2_size = 0;
};

struct Header
{
	RIFF riff;
	FMT fmt;
	DATA data;
};

/* ���������� */
struct CoreDecoder {
	int16_t sample_val = 0;             /* ����ֵ               */
	int8_t  stepsize_index = 0;         /* ������Ĳο�����     */
};

/* ���������� */
struct CoreEncoder {
	int16_t prev_sample = 0;            /* ����ֵ               */
	int8_t  stepsize_index = 0;         /* ������Ĳο�����     */
};

Ima_Adpcm_Engine::Ima_Adpcm_Engine()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	m_header_ptr = std::make_shared<Header>();

	struct tm t;
	localtime_s(&t, &in_time_t);
	std::stringstream ss;
	ss << "./" << t.tm_year + 1900 << "��" << t.tm_mon << "��" << t.tm_mday << "��" << t.tm_hour << "ʱ" << t.tm_min << "��" << t.tm_sec << "��.wav";
	std::string file_name = ss.str();

	// ���������½��ļ�
	std::ofstream tmp_file;
	tmp_file.open(file_name, std::ios::app);
	tmp_file.clear();
	tmp_file.close();

	m_out_ptr = std::make_shared<std::fstream>(file_name, std::ios::binary | std::ios::out | std::ios::in);

	uint16_t size = sizeof *m_header_ptr;

	m_out_ptr->write((const char*)m_header_ptr.get(), sizeof * m_header_ptr);

	for (int i = 0 ; i < 2 ; ++i)
	{
		std::shared_ptr<CoreDecoder> decoder_ptr = std::make_shared<CoreDecoder>();
		std::shared_ptr<CoreEncoder> eecoder_ptr = std::make_shared<CoreEncoder>();

		m_decoder_ptr[i] = decoder_ptr;
		m_encoder_ptr[i] = eecoder_ptr;
	}
}

Ima_Adpcm_Engine::Ima_Adpcm_Engine(std::string file_name)
{
	// ���������½��ļ�
	std::ofstream tmp_file;
	tmp_file.open(file_name, std::ios::app);
	tmp_file.clear();
	tmp_file.close();

	m_out_ptr = std::make_shared<std::fstream>(file_name, std::ios::binary | std::ios::out | std::ios::in);

	uint16_t size = sizeof * m_header_ptr;

	m_out_ptr->write((const char*)m_header_ptr.get(), sizeof * m_header_ptr);

	for (int i = 0; i < 2; ++i)
	{
		std::shared_ptr<CoreDecoder> decoder_ptr = std::make_shared<CoreDecoder>();
		std::shared_ptr<CoreEncoder> eecoder_ptr = std::make_shared<CoreEncoder>();

		m_decoder_ptr[i] = decoder_ptr;
		m_encoder_ptr[i] = eecoder_ptr;
	}
}

Ima_Adpcm_Engine::~Ima_Adpcm_Engine()
{
	uint16_t size = sizeof * m_header_ptr;

	m_out_ptr->seekp(0, std::ios::beg);

	m_out_ptr->write((const char*)m_header_ptr.get(), size);

	m_out_ptr->close();
}

int Ima_Adpcm_Engine::encoder(std::array<std::vector<int16_t>, 2>& pcm, std::vector<uint8_t>& ima, uint16_t num_channels)
{
	int ret;

 	/* ����� */
	ret = ima_adpcm_encoder_block(pcm, ima, num_channels);
		
	return ret;
}

int Ima_Adpcm_Engine::decode(std::vector<uint8_t>& ima, std::array<std::vector<int16_t>, 2>& pcm, uint16_t num_channels)
{
	int ret;

	/* ����� */
	ret = ima_adpcm_decode_block(ima, pcm, num_channels);

	return ret;
}

int16_t Ima_Adpcm_Engine::ima_adpcm_decoder_sample(std::shared_ptr<CoreDecoder> decoder_ptr, uint8_t nibble)
{
	int8_t  idx;
	int32_t predict, qdiff, delta, stepsize;

	assert(decoder_ptr != nullptr);

	/* ����Ƶ���ο��ı���Ϊ�Զ����� */
	predict = decoder_ptr->sample_val;
	idx = decoder_ptr->stepsize_index;

	/* �����Ļ�ȡ */
	stepsize = stepsize_table[idx];

	/* �������� */
	idx = (int8_t)(idx + index_table[nibble]);
	idx = IMAADPCM_INNER_VAL(idx, 0, 88);

	/* ��ּ��� */
	/* diff = stepsize * (delta * 2 + 1) / 8 */
	/* memo:ffmpeg ���ο��������ķ�֧���õ�ʵ�ֲ�����
	 * �жϷ�֧���õ�ʵ�ֽ��ƽ����֣���֧����ĸ��ɸ��� */
	delta = nibble & 7;
	qdiff = (stepsize * ((delta << 1) + 1)) >> 3;

	/* �üӲ����λ�л��ӷ�/���� */
	if (nibble & 8) {
		predict -= qdiff;
	}
	else {
		predict += qdiff;
	}

	/* 16bit��Ȼ����� */
	predict = IMAADPCM_INNER_VAL(predict, -32768, 32767);

	/* �������ķ�ӳ */
	decoder_ptr->sample_val = (int16_t)predict;
	decoder_ptr->stepsize_index = idx;

	return decoder_ptr->sample_val;
}

uint8_t Ima_Adpcm_Engine::ima_adpcm_encoder_sample(std::shared_ptr<CoreEncoder> encoder_ptr, int16_t sample)
{
	uint8_t nibble;
	int8_t idx;
	int32_t prev, diff, qdiff, delta, stepsize, diffabs, sign;

	assert(encoder_ptr != nullptr);

	/* ���պͽ��ղο��������Զ����� */
	prev = encoder_ptr->prev_sample;
	idx = encoder_ptr->stepsize_index;

	/* �����Ļ�ȡ */
	stepsize = stepsize_table[idx];

	/* ��� */
	diff = sample - prev;
	sign = diff < 0;
	diffabs = sign ? -diff : diff;

	/* ������ת��Ϊ�з��ű�ʾ */
	/* nibble = sign(diff) * round(|diff| * 4 / stepsize) */
	nibble = (uint8_t)IMAADPCM_MIN_VAL((diffabs << 2) / stepsize, 7);
	/* nibble �����λ�Ƿ���λ */
	if (sign) {
		nibble |= 0x8;
	}

	/* ������������ */
	delta = nibble & 7;
	qdiff = (stepsize * ((delta << 1) + 1)) >> 3;

	/* ��������������� */
	/* printf("%d \n", sign ? (-qdiff - diff) : (qdiff - diff)); */

	/* ����������� */
	if (sign) {
		prev -= qdiff;
	}
	else {
		prev += qdiff;
	}
	prev = IMAADPCM_INNER_VAL(prev, -32768, 32767);

	/* �������� */
	idx = (int8_t)(idx + index_table[nibble]);
	idx = IMAADPCM_INNER_VAL(idx, 0, 88);

	/* �������ķ��� */
	encoder_ptr->prev_sample = (int16_t)prev;
	encoder_ptr->stepsize_index = idx;

	return nibble;
}

int Ima_Adpcm_Engine::ima_adpcm_decode_block_mono(std::shared_ptr<CoreDecoder> decoder_ptr, std::vector<uint8_t>& ima, std::vector<int16_t>& pcm)
{
	uint8_t u8buf;
	uint8_t nibble[2];
	uint32_t smpl, tmp_num_decode_samples;

	/* ������� */
	if ((decoder_ptr == nullptr) || (ima.empty()) || (pcm.empty())) {
		return -1;
	}

	/* ����ɽ��������� */
	tmp_num_decode_samples = (256 - 4) * 2;
	tmp_num_decode_samples += 1;

	auto it = ima.begin();

	/* ��ͷ���� */
	ByteArray_GetUint16LE(it, (uint16_t*)&(decoder_ptr->sample_val));
	ByteArray_GetUint8(it, (uint8_t*)&(decoder_ptr->stepsize_index));
	ByteArray_GetUint8(it, &u8buf); /* ���� */
	if (u8buf != 0) {
		return -1;
	}

	/* ��һ������ */
	pcm[0] = decoder_ptr->sample_val;

	/* �����ݽ��� */
	for (smpl = 1; smpl < tmp_num_decode_samples; smpl += 2) {
		ByteArray_GetUint8(it, &u8buf);
		nibble[0] = (u8buf >> 0) & 0xF;
		nibble[1] = (u8buf >> 4) & 0xF;
		ima[smpl + 0] = ima_adpcm_decoder_sample(decoder_ptr, nibble[0]);
		ima[smpl + 1] = ima_adpcm_decoder_sample(decoder_ptr, nibble[1]);
	}

	return 0;
}

int Ima_Adpcm_Engine::ima_adpcm_decode_block_stereo(std::array<std::shared_ptr<CoreDecoder>, 2> decoder_ptr, std::vector<uint8_t>& ima, std::array<std::vector<int16_t>, 2>& pcm)
{
	uint32_t u32buf;
	uint8_t nibble[8];
	uint32_t ch, smpl, tmp_num_decode_samples;

	/* �Ա������ */
	if ((decoder_ptr[0] == nullptr) || (decoder_ptr[1] == nullptr) || (ima.empty()) || (pcm.empty())) {
		return -1;
	}

	/* ����ɽ�������������+1��ͷ������ */
	tmp_num_decode_samples = 512 - 8;
	tmp_num_decode_samples += 1;

	auto it = ima.begin();

	/* ��ͷ���� */
	for (ch = 0; ch < 2; ch++) {
		uint8_t reserved;
		ByteArray_GetUint16LE(it, (uint16_t*)&(decoder_ptr[ch]->sample_val));
		ByteArray_GetUint8(it, (uint8_t*)&(decoder_ptr[ch]->stepsize_index));
		ByteArray_GetUint8(it, &reserved);
		if (reserved != 0) {
			return -1;
		}
	}

	/* ��һ�������Ļ�ȡ */
	for (ch = 0; ch < 2; ch++) {
		pcm[ch][0] = decoder_ptr[ch]->sample_val;
	}

	/* �����ݽ��� */
	for (smpl = 1; smpl < tmp_num_decode_samples; smpl += 8) {
		uint32_t smp;
		int16_t  buf[8];
		for (ch = 0; ch < 2; ch++) {
			ByteArray_GetUint32LE(it, &u32buf);
			nibble[0] = (uint8_t)((u32buf >> 0) & 0xF);
			nibble[1] = (uint8_t)((u32buf >> 4) & 0xF);
			nibble[2] = (uint8_t)((u32buf >> 8) & 0xF);
			nibble[3] = (uint8_t)((u32buf >> 12) & 0xF);
			nibble[4] = (uint8_t)((u32buf >> 16) & 0xF);
			nibble[5] = (uint8_t)((u32buf >> 20) & 0xF);
			nibble[6] = (uint8_t)((u32buf >> 24) & 0xF);
			nibble[7] = (uint8_t)((u32buf >> 28) & 0xF);

			/* ��Ϊ�����������ܲ���1 +(8�ı���)��������ʱ�ܵ������� */
			buf[0] = ima_adpcm_decoder_sample(decoder_ptr[ch], nibble[0]);
			buf[1] = ima_adpcm_decoder_sample(decoder_ptr[ch], nibble[1]);
			buf[2] = ima_adpcm_decoder_sample(decoder_ptr[ch], nibble[2]);
			buf[3] = ima_adpcm_decoder_sample(decoder_ptr[ch], nibble[3]);
			buf[4] = ima_adpcm_decoder_sample(decoder_ptr[ch], nibble[4]);
			buf[5] = ima_adpcm_decoder_sample(decoder_ptr[ch], nibble[5]);
			buf[6] = ima_adpcm_decoder_sample(decoder_ptr[ch], nibble[6]);
			buf[7] = ima_adpcm_decoder_sample(decoder_ptr[ch], nibble[7]);
			for (smp = 0; (smp < 8) && ((smpl + smp) < tmp_num_decode_samples); smp++) {
				pcm[ch][smpl + smp] = buf[smp];
			}
		}
	}

	return 0;
}

int Ima_Adpcm_Engine::ima_adpcm_core_encoder_block_mono(std::shared_ptr<CoreEncoder> encoder_ptr, std::vector<int16_t>& pcm, std::vector<uint8_t>& ima)
{
	uint8_t u8buf;
	uint8_t nibble[2];
	uint32_t smpl;

	/* ������� */
	if ((encoder_ptr == nullptr) || (pcm.empty()) || (ima.empty())) {
		return -1;
	}

	/* �ڱ����������õ�һ������ */
	encoder_ptr->prev_sample = pcm.front();

	auto it = ima.begin();

	/* ��ͷ���� */
	ByteArray_PutUint16LE(it, encoder_ptr->prev_sample);
	ByteArray_PutUint8(it, encoder_ptr->stepsize_index);
	ByteArray_PutUint8(it, 0); /* ���� */

	/* �����ݱ��� */
	for (smpl = 1; smpl < 505; smpl += 2) {
		nibble[0] = ima_adpcm_encoder_sample(encoder_ptr, pcm[smpl + 0]);
		nibble[1] = ima_adpcm_encoder_sample(encoder_ptr, pcm[smpl + 1]);
		assert((nibble[0] <= 0xF) && (nibble[1] <= 0xF));
		u8buf = (uint8_t)((nibble[0] << 0) | (nibble[1] << 4));
		ByteArray_PutUint8(it, u8buf);
	}

	return 0;
}

int Ima_Adpcm_Engine::ima_adpcm_core_encoder_block_stereo(std::array<std::shared_ptr<CoreEncoder>, 2> encoder_ptr, std::array<std::vector<int16_t>, 2>& pcm, std::vector<uint8_t>& ima)
{
	uint32_t u32buf;
	uint8_t nibble[8];
	uint32_t ch, smpl;

	/* ������� */
	if ((encoder_ptr[0] == nullptr) || (encoder_ptr[1] == nullptr) || (pcm.empty()) || (ima.empty())) {
		return -1;
	}

	/* �ڱ����������õ�һ������ */
	for (ch = 0; ch < 2; ch++) {
		encoder_ptr[ch]->prev_sample = pcm[ch][0];
	}

	auto it = ima.begin();

	/* ��ͷ���� */
	for (ch = 0; ch < 2; ch++) {
		ByteArray_PutUint16LE(it, encoder_ptr[ch]->prev_sample);
		ByteArray_PutUint8(it, encoder_ptr[ch]->stepsize_index);
		ByteArray_PutUint8(it, 0); /* ���� */
	}

	/* �����ݱ��� */
	for (smpl = 1; smpl < 505; smpl += 8) {
		for (ch = 0; ch < 2; ch++) {
			nibble[0] = ima_adpcm_encoder_sample(encoder_ptr[ch], pcm[ch][smpl + 0]);
			nibble[1] = ima_adpcm_encoder_sample(encoder_ptr[ch], pcm[ch][smpl + 1]);
			nibble[2] = ima_adpcm_encoder_sample(encoder_ptr[ch], pcm[ch][smpl + 2]);
			nibble[3] = ima_adpcm_encoder_sample(encoder_ptr[ch], pcm[ch][smpl + 3]);
			nibble[4] = ima_adpcm_encoder_sample(encoder_ptr[ch], pcm[ch][smpl + 4]);
			nibble[5] = ima_adpcm_encoder_sample(encoder_ptr[ch], pcm[ch][smpl + 5]);
			nibble[6] = ima_adpcm_encoder_sample(encoder_ptr[ch], pcm[ch][smpl + 6]);
			nibble[7] = ima_adpcm_encoder_sample(encoder_ptr[ch], pcm[ch][smpl + 7]);
			assert((nibble[0] <= 0xF) && (nibble[1] <= 0xF) && (nibble[2] <= 0xF) && (nibble[3] <= 0xF)
				&& (nibble[4] <= 0xF) && (nibble[5] <= 0xF) && (nibble[6] <= 0xF) && (nibble[7] <= 0xF));
			u32buf = (uint32_t)(nibble[0] << 0);
			u32buf |= (uint32_t)(nibble[1] << 4);
			u32buf |= (uint32_t)(nibble[2] << 8);
			u32buf |= (uint32_t)(nibble[3] << 12);
			u32buf |= (uint32_t)(nibble[4] << 16);
			u32buf |= (uint32_t)(nibble[5] << 20);
			u32buf |= (uint32_t)(nibble[6] << 24);
			u32buf |= (uint32_t)(nibble[7] << 28);
			ByteArray_PutUint32LE(it, u32buf);
		}
	}

	return 0;
}

int Ima_Adpcm_Engine::ima_adpcm_decode_block(std::vector<uint8_t>& ima, std::array<std::vector<int16_t>, 2>& pcm, uint16_t num_channels)
{
	// �������
	if ((ima.empty()) || (pcm.empty())) {
		return -1;
	}

	/* ����� */
	switch (num_channels) {
	case 1:
		ima_adpcm_decode_block_mono(m_decoder_ptr[0], ima, pcm[0]);
		break;
	case 2:
		ima_adpcm_decode_block_stereo(m_decoder_ptr, ima, pcm);
		break;
	default:
		return -1;
	}

	return 0;
}

int Ima_Adpcm_Engine::ima_adpcm_encoder_block(std::array<std::vector<int16_t>, 2>& pcm, std::vector<uint8_t>& ima, uint16_t num_channels)
{
	/* ������� */
	if ((pcm.empty())) {
		return -1;
	}

	/* ����� */
	switch (num_channels) {
	case 1:
		ima_adpcm_core_encoder_block_mono(m_encoder_ptr[0], pcm[0], ima);
		break;
	case 2:
		ima_adpcm_core_encoder_block_stereo(m_encoder_ptr, pcm, ima);
		break;
	default:
		return -1;
	}

	m_out_ptr->write((const char*)ima.data(), ima.size());

	m_header_ptr->data.sub_chunk2_size += ima.size();
	m_header_ptr->riff.chunk_size += ima.size();

	return 0;
}


