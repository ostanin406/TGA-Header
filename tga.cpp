#include <cstdio>
#include <cstdint>
#include <cstdlib>

uint8_t SaveTGA(const char* file, uint16_t width, uint16_t height, uint8_t* data, uint8_t depth)
{
	if (!width || !height || !data || !(depth == 24 || depth == 32))
		return 0;

	FILE* f = NULL;
	if (fopen_s(&f, file, "wb") || !f)
		return 0;

	// header[2] = RLE ? 10 : 2
	uint8_t header[18] = { 0,0,2,0,0,0,0,0,0,0,0,0,
		(uint8_t)(width % 256), (uint8_t)(width / 256),
		(uint8_t)(height % 256), (uint8_t)(height / 256),
		depth, (uint8_t)((depth == 32) ? 8 : 0) };

	fwrite(&header, sizeof(header) / sizeof(uint8_t), 1, f);

	uint32_t pixnum = width * height;
	uint8_t channel = depth / 0x08;

	for (uint32_t p = 0; p < pixnum; p++)
		for (uint8_t c = 0; c < channel; c++)
			fputc(data[(p * channel) + c], f);

	fclose(f);
	return 1;
}

// pitch = (depth / 8) * width
#pragma warning(push)
#pragma warning(disable:6386)
uint8_t* LoadTGA(const char* file, uint16_t& width, uint16_t& height, uint8_t& depth)
{
	FILE* f = NULL;
	if (fopen_s(&f, file, "rb") || !f)
		return 0;

	fseek(f, 2L, SEEK_SET);
	if (fgetc(f) == 10) goto gotoerrno; // RLE?

	fseek(f, 12L, SEEK_SET);
	fread(&width, sizeof(uint16_t), 1, f);
	fread(&height, sizeof(uint16_t), 1, f);
	depth = fgetc(f); // BPP
	fseek(f, 1L, SEEK_CUR); // alpha bits

	if (width && height && (depth == 24 || depth == 32))
	{
		uint32_t pixnum = width * height;
		uint8_t channel = depth / 0x08;
		uint8_t* data = (uint8_t*)malloc(pixnum * channel * sizeof(uint8_t));
		if (!data) goto gotoerrno;

		for (uint32_t p = 0; p < pixnum; p++)
			for (uint8_t c = 0; c < channel; c++)
				data[p * channel + c] = fgetc(f);

		fclose(f);
		return data;
	}

	gotoerrno:
	fclose(f);
	return 0;
}
#pragma warning(pop)

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("[Error]: None file *.tga\n");
		system("pause");
		return 0;
	}

	uint8_t depth;
	uint16_t width, height;
	uint8_t* data = LoadTGA(argv[1], width, height, depth);

	if (!data)
	{
		printf("[Error]: Load file %s\n", argv[1]);
		system("pause");
		return 0;
	}

	printf("Width = %i\n", width);
	printf("Height = %i\n", height);
	printf("Depth = %i\n", depth);

	if (!SaveTGA("new.tga", width, height, data, depth))
	{
		printf("[Error]: Save file\n");
		system("pause");
		return 0;
	}

	system("pause");
	return 0;
}