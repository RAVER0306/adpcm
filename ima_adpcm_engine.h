#pragma once

#include <fstream>
#include <memory>
#include <array>
#include <vector>

enum class Format
{
	FORMAT_IMA_ADPCM = 0x11		// ima adpcm
};

struct CoreEncoder;
struct CoreDecoder;
struct Header;

class Ima_Adpcm_Engine
{
public:
	explicit Ima_Adpcm_Engine();
	explicit Ima_Adpcm_Engine(std::string file_name);
	~Ima_Adpcm_Engine();

	/* ���� */
	int encoder(std::array<std::vector<int16_t>, 2>& pcm, std::vector<uint8_t>& ima, uint16_t num_channels);

	/* ���� */
	int decode(std::vector<uint8_t>& ima, std::array<std::vector<int16_t>, 2>& pcm, uint16_t num_channels);

private:
	/* �������� */
	static int16_t ima_adpcm_decoder_sample(std::shared_ptr<CoreDecoder> decoder_ptr, uint8_t nibble);

	/* �������� */
	static uint8_t ima_adpcm_encoder_sample(std::shared_ptr<CoreEncoder> encoder_ptr, int16_t sample);

	/* ����������� */
	static int ima_adpcm_decode_block_mono(std::shared_ptr<CoreDecoder> decoder_ptr, std::vector<uint8_t>& ima, std::vector<int16_t>& pcm);

	/* ����������� */
	static int ima_adpcm_decode_block_stereo(std::array<std::shared_ptr<CoreDecoder>, 2> decoder_ptr, std::vector<uint8_t>& ima, std::array<std::vector<int16_t>, 2>& pcm);

	/* ����������� */
	static int ima_adpcm_core_encoder_block_mono(std::shared_ptr<CoreEncoder> encoder_ptr, std::vector<int16_t>& pcm, std::vector<uint8_t>& ima);

	/* ����������� */
	static int ima_adpcm_core_encoder_block_stereo(std::array<std::shared_ptr<CoreEncoder>, 2> encoder_ptr, std::array<std::vector<int16_t>, 2>& pcm, std::vector<uint8_t>& ima);

	/* �����ݿ���� */
	int ima_adpcm_decode_block(std::vector<uint8_t>& ima, std::array<std::vector<int16_t>, 2>& pcm, uint16_t num_channels);
	
	/* �����ݿ���� */
	int ima_adpcm_encoder_block(std::array<std::vector<int16_t>, 2>& pcm, std::vector<uint8_t>& ima, uint16_t num_channels);

private:
	std::array<std::shared_ptr<CoreDecoder>, 2> m_decoder_ptr;
	std::array<std::shared_ptr<CoreEncoder>, 2> m_encoder_ptr;
	std::shared_ptr<Header> m_header_ptr;
	std::shared_ptr<std::fstream> m_out_ptr;
};