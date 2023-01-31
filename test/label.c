/* Generates label image and a json (blob.json) along a GNUplot compatible
 * (blob.plot) files containing the set of extracted blobs.
 * The GNUplot file can be plotted with :
 *     plot "blob.plot" lc variable with lines         
 *
 * Licensed under the MIT License
 * (c) 2016-2023 Vincent Cruz
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <getopt.h>

#include "stb_image.h"
#include "stb_image_write.h"

#include <blob.h>

/* Output label buffer as an RGB PNG image */
void label_write_png(label_t *label, int width, int height, const char *filename)
{
    static const uint8_t palette[8][3] = 
    {
        {0xff,0x00,0x00},
        {0x00,0xff,0x00},
        {0xff,0xff,0x00},
        {0x00,0x00,0xff},
        {0xff,0x00,0xff},
        {0x00,0xff,0xff},
        {0xff,0xff,0xff},
        {0x7f,0x00,0x7f}
    };
    static const int palette_len = sizeof(palette) / sizeof(palette[0]);
    
    int i;
    uint8_t *out;
    uint8_t *ptr;
    
    out = (uint8_t*)malloc(3 * width * height * sizeof(uint8_t));
    
    for(i=width*height, ptr=out; i>0; i--, label++, ptr+=3)
    {
        if(*label > 0)
        {
            int index = (*label - 1) % palette_len;
            ptr[0] = palette[index][0];
            ptr[1] = palette[index][1];
            ptr[2] = palette[index][2];
        }
        else
        {
            ptr[0] = ptr[1] = ptr[2] = 0;
        }
    }

    if( !stbi_write_png(filename, width, height, 3, out, 0) )
    {
        fprintf(stderr, "failed to write %s\n", out);
    }
    
    free(out);
}

/* write contour as JSON element */
void contour_write_json(contour_t *contour, const char *name, int depth, FILE *out)
{
    int i;
    int16_t *ptr = contour->points;
    char tab[16];
    
    memset(tab, ' ', depth*2);
    tab[depth*2] = '\0';
    
    fprintf(out, "%s", tab);
    if(NULL != name)
    {
        fprintf(out, "\"%s\" : ", name);
    }    
    fprintf(out, "[\n");
    for(i=contour->count; i>0; )
    {
        int j;
        fprintf(out, "%s  ", tab);
        for(j=0; (j<8) && (i>0); j++, i--, ptr+=2)
        {
            fprintf(out, "%5d, %5d%c", ptr[0], ptr[1], (i>1) ? ',' : ' ');
        }
        fprintf(out, "\n");
    }
}

/* write blobs in a JSON file */
int blob_write_json(blob_t *blobs, int count, const char *filename)
{
    int i;
    FILE *out = fopen(filename, "wb");
    if(NULL == out)
    {
        fprintf(stderr, "failed to open %s : %s\n", filename, strerror(errno));
        return 0;
    }
    fprintf(out, "{\n  \"blobs\" : [\n");
    for(i=0; i<count; i++)
    {
        fprintf(out, "    {\n");
        fprintf(out, "      \"label\" : %d,\n", blobs[i].label);    
        contour_write_json(&blobs[i].external, "external", 3, out);
        fprintf(out, ",\n");
        if(NULL != blobs[i].internal)
        {
            int j;
            fprintf(out, "      \"internals\" : [\n");
            for(j=0; j<blobs[i].internal_count; j++)
            {
                contour_write_json(blobs[i].internal+j, NULL, 4, out);
                fprintf(out, "%c\n", (j < (blobs[i].internal_count-1)) ? ',' : ' ');
            }
            fprintf(out, "      ],\n");
        }
        fprintf(out, "      \"euler_number\": %d\n", blobs[i].internal_count);
        fprintf(out, "    }%c\n", (i < (count-1)) ? ',' : ' ');
    }
    fprintf(out, "  ]\n}\n");
    fclose(out);
    return 1;
}

/* write contour as GNUplot data */
void contour_write_plot(contour_t *contour, int16_t label, FILE *out)
{
    int i;
    int16_t *ptr = contour->points;

    for(i=contour->count; i>0; i--, ptr+=2)
    {
        fprintf(out, "%5d    %5d    %5d\n", ptr[0], ptr[1], label);
    }
}

/* write blob contours in a GNUplot file */
int blob_write_plot(blob_t *blobs, int count, const char *filename)
{
    int i;
    FILE *out = fopen(filename, "wb");
    if(NULL == out)
    {
        fprintf(stderr, "failed to open %s : %s\n", filename, strerror(errno));
        return 0;
    }

    for(i=0; i<count; i++)
    {
        contour_write_plot(&blobs[i].external, 2 * blobs[i].label, out);
        fprintf(out, "\n");
        if(NULL != blobs[i].internal)
        {
            int j;
            for(j=0; j<blobs[i].internal_count; j++)
            {
                contour_write_plot(blobs[i].internal + j,  (2 * blobs[i].label) + 1, out);
                fprintf(out, "\n");
            }
        }
    }
    fclose(out);
    return 1;
}

/* simple dummy threshold */
void threshold(uint8_t *source, int width, int height, uint8_t v)
{
    int i;
    for(i=width*height; i>0; i--)
    {
        uint8_t c = (*source >= v) ? 1 : 0;
        *source++ = c;
    }
}

void usage()
{
    fprintf(stderr, "Usage : label [options] <in> <out>\n"
            "Create an image containing the set of found labels and a "
            "json file (blob.json) and GNUplot file (blob.plot) "
            "containing the associated blobs informations.\n"
            "--roi_x or -x : X coordinate of the upper left corner of ROI (default: 0).\n"
            "--roi_y or -y : Y coordinate of the upper left corner of ROI (default: 0).\n"
            "--roi_w or -w : width of the ROI (default: input image width).\n"
            "--roi_h or -h : height of the ROI (default: input image height).\n"
            "--help or -h  : displays this message.\n"
            "<in>          : input image.\n"
            "<out>         : output label image.\n");
}

int main(int argc, char **argv)
{
    int ret;
    
    int width  = 0;
    int height = 0;
    uint8_t *image = NULL;
    
    label_t *label  = NULL;
    int16_t label_width = 0;
    int16_t label_height = 0;
    
    blob_t *blobs;
    int blob_count;

    int16_t roi_x, roi_y, roi_w, roi_h;

    char *short_options = "x:y:w:h:?";
    struct option long_options[] = {
        {"roi_x", 1, 0, 'x'},
        {"roi_y", 1, 0, 'y'},
        {"roi_w", 1, 0, 'w'},
        {"roi_h", 1, 0, 'h'},
        {"help",  0, 0, '?'},
        { 0,      0, 0,  0 }
    };
    int idx, opt;

    roi_x = roi_y = 0;
    roi_w = roi_h = -1;

    while ((opt = getopt_long (argc, argv, short_options, long_options, &idx)) > 0)
    {
        switch(opt)
        {
            case 'x':
                roi_x = atoi(optarg);
                break;
            case 'y':
                roi_y = atoi(optarg);
                break;
            case 'w':
                roi_w = atoi(optarg);
                break;
            case 'h':
                roi_h = atoi(optarg);
                break;
            case '?':
                usage();
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "error: invalid option\n");
                usage();
                return EXIT_FAILURE;
        }
    }

    if((argc - optind) != 2)
    {
        fprintf(stderr, "error: missing parameters\n");
        usage();
        return EXIT_FAILURE;
    }
    
    image = stbi_load(argv[optind], &width, &height, NULL, 1);
    if(NULL == image)
    {
        fprintf(stderr, "failed to read image : %s\n", argv[optind]);
        return EXIT_FAILURE;
    }

    threshold(image, width, height, 128); 

    blobs = NULL;
    blob_count = 0;
    
    if(roi_w < 0) { roi_w = width; }
    if(roi_h < 0) { roi_h = height; }
    
    ret = EXIT_FAILURE;
    
    if( find_blobs(roi_x, roi_y, roi_w, roi_h, image, width, height, &label, &label_width, &label_height, &blobs, &blob_count, 1) )
    {
        label_write_png(label, label_width, label_height, argv[optind+1]);
        if(    blob_write_json(blobs, blob_count, "blob.json")
            && blob_write_plot(blobs, blob_count, "blob.plot") )
        {
            ret = EXIT_SUCCESS;
        }
    }
    
    destroy_blobs(blobs, blob_count);
    
    if(NULL != label)
    {
        free(label);
    }

    free(image);

    return ret;
}
