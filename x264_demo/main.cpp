
#include <stdint.h>
extern "C" {
#include "x264.h"
#include "x264_config.h"
}

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char **argv)
{
    int width, height;
    x264_param_t param;
    x264_picture_t pic;
    x264_picture_t pic_out;
    x264_t *h;
    int i_frame = 0;
    int i_frame_size;
    x264_nal_t *nal;
    int i_nal;

    width = 480;
    height = 272;

    std::string input_file("../foreman_480x272.yuv");
    std::ifstream fin(input_file, std::ios::binary);
    if (!fin.is_open()) {
        std::cerr << "Can't open file: " << input_file << std::endl;
        return -1;
    }

    std::string output_file("../output.h264");
    std::ofstream fout(output_file, std::ios::binary);
    if (!fout.is_open()) {
        std::cerr << "Can't open file: " << output_file << std::endl;
        return -1;
    }

    /* Get default params for preset/tuning */
    if (x264_param_default_preset(&param, "medium", NULL) < 0) {
        std::cerr << "x264_param_default_preset error:" << std::endl;
        return -1;
    }
    /* Configure non-default params */
    param.i_bitdepth = 8;
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height = height;
    param.b_vfr_input = 0;
    param.b_repeat_headers = 1;
    param.b_annexb = 1;

    /* Apply profile restrictions. */
    if (x264_param_apply_profile(&param, "high") < 0) {
        std::cerr << "x264_param_apply_profile error." << std::endl;
        return -1;
    }

    if (x264_picture_alloc(&pic, param.i_csp, param.i_width, param.i_height) < 0) {
        std::cerr << "x264_picture_alloc" << std::endl;
        return -1;
    }

    h = x264_encoder_open(&param);
    if (!h) {
        std::cerr << "x264_encoder_open error" << std::endl;
        x264_picture_clean(&pic);
        return -1;
    }

    int luma_size = width * height;
    int chroma_size = luma_size / 4;
    /* Encode frames */
    for (;; i_frame++)
    {
        /* Read input frame */
        fin.read(reinterpret_cast<char*>(pic.img.plane[0]), luma_size);
        if (!fin) {
            break;
        }
        fin.read(reinterpret_cast<char*>(pic.img.plane[1]), chroma_size);
        if (!fin) {
            break;
        }
        fin.read(reinterpret_cast<char*>(pic.img.plane[2]), chroma_size);
        if (!fin) {
            break;
        }

        pic.i_pts = i_frame;
        i_frame_size = x264_encoder_encode(h, &nal, &i_nal, &pic, &pic_out);
        if (i_frame_size < 0) {
            x264_encoder_close(h);
            x264_picture_clean(&pic);
            return -1;
        }
        else if (i_frame_size)
        {
            fout.write(reinterpret_cast<char*>(nal->p_payload), i_frame_size);
        }
    }
    /* Flush delayed frames */
    while (x264_encoder_delayed_frames(h))
    {
        i_frame_size = x264_encoder_encode(h, &nal, &i_nal, NULL, &pic_out);
        if (i_frame_size < 0) {
            x264_encoder_close(h);
            x264_picture_clean(&pic);
            return -1;
        }
        else if (i_frame_size)
        {
            fout.write(reinterpret_cast<char*>(nal->p_payload), i_frame_size);
        }
    }

    x264_encoder_close(h);
    x264_picture_clean(&pic);
    fin.close();
    fout.close();
    return 0;
}
