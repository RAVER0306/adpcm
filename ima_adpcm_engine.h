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

	/* 编码 */
	int encoder(std::array<std::vector<int16_t>, 2>& pcm, std::vector<uint8_t>& ima, uint16_t num_channels);

	/* 解码 */
	int decode(std::vector<uint8_t>& ima, std::array<std::vector<int16_t>, 2>& pcm, uint16_t num_channels);

private:
	/* 样本解码 */
	static int16_t ima_adpcm_decoder_sample(std::shared_ptr<CoreDecoder> decoder_ptr, uint8_t nibble);

	/* 样本编码 */
	static uint8_t ima_adpcm_encoder_sample(std::shared_ptr<CoreEncoder> encoder_ptr, int16_t sample);

	/* 单声道块解码 */
	static int ima_adpcm_decode_block_mono(std::shared_ptr<CoreDecoder> decoder_ptr, std::vector<uint8_t>& ima, std::vector<int16_t>& pcm);

	/* 立体声块解码 */
	static int ima_adpcm_decode_block_stereo(std::array<std::shared_ptr<CoreDecoder>, 2> decoder_ptr, std::vector<uint8_t>& ima, std::array<std::vector<int16_t>, 2>& pcm);

	/* 单声道块编码 */
	static int ima_adpcm_core_encoder_block_mono(std::shared_ptr<CoreEncoder> encoder_ptr, std::vector<int16_t>& pcm, std::vector<uint8_t>& ima);

	/* 立体声块编码 */
	static int ima_adpcm_core_encoder_block_stereo(std::array<std::shared_ptr<CoreEncoder>, 2> encoder_ptr, std::array<std::vector<int16_t>, 2>& pcm, std::vector<uint8_t>& ima);

	/* 单数据块解码 */
	int ima_adpcm_decode_block(std::vector<uint8_t>& ima, std::array<std::vector<int16_t>, 2>& pcm, uint16_t num_channels);
	
	/* 单数据块编码 */
	int ima_adpcm_encoder_block(std::array<std::vector<int16_t>, 2>& pcm, std::vector<uint8_t>& ima, uint16_t num_channels);

private:
	std::array<std::shared_ptr<CoreDecoder>, 2> m_decoder_ptr;
	std::array<std::shared_ptr<CoreEncoder>, 2> m_encoder_ptr;
	std::shared_ptr<Header> m_header_ptr;
	std::shared_ptr<std::fstream> m_out_ptr;
};