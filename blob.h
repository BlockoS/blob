/**
 * \mainpage
 * blob - 8-neighbour connected components labelling and contours extractor
 * =========================================================================
 * v0.0.1
 * Licensed under the MIT License
 * (c) 2016-2023 Vincent Cruz
 * 
 * About:
 * ------
 * This header file provides a function to label and extract the contours
 * of connected components (8-connected) from a region of an image.
 * The implementation is based upon the following paper:
 * 
 * "A linear-time component-labeling algorithm using contour tracing technique"
 *  by Fu Chang, Chun-Jen Chen, and Chi-Jen Lu.
 * 
 * Building:
 * ---------
 * Before include this file, add the following line
 ```
 *     #define BLOB_IMPLEMENTATION
 ```
 * in the file where you want to have the implementation.
 * 
 * You can define `BLOB_MALLOC`, `BLOB_REALLOC` and `BLOB_FREE` before the
 * include to replace `malloc`, `realloc` and `free`.
 * 
 * You can define `BLOB_MEMSET` to replace memset.
 * 
 * Errors messages (out of memory, invalid arguments) are displayed via
 * `BLOB_ERROR`. By default this macro uses fprintf (hence adding a
 * dependency to stdio.h). `BLOB_ERROR` can be defined to replace the
 * default behaviour.
 * 
 * Usage:
 * ------
 ```
 * int find_blobs( int16_t roi_x, int16_t roi_y, int16_t roi_w, int16_t roi_h,
 *                 uint8_t *in, int16_t in_w, int16_t in_h, 
 *                 label_t **label, int16_t *label_w, int16_t *label_h, 
 *                 blob_t** blobs, int *count, int extract_internal );
 ```
 *  
 * The first 4 parameters `roi_x`, `roi_y`, `roi_w`, `roi_h` defines the
 * area in the input image where the labels and contours will be extracted.
 * This area may be clamped to the input image dimension.
 * 
 * `in`, `in_w`, `in_h` are the pointer to the input image and its dimensions.
 * Note that the input image must be a 8bpp grayscale image. All pixels
 * with a value of 0 are considered to be part of the background. All
 * other values are treated as being part of the background.
 * 
 * `label`, `label_w`, `label_h` are the pointer to the label buffer and its
 * dimensions. The dimensions of the label buffer may be equal to `roi_w`
 * and `roi_h`. A pixel in the label buffer may be equal to the label of
 * the blob it belongs to, 0 for background and -1 for background pixels
 * near blob countours.
 * 
 * `blobs` and `count` are respectively the array and the number of
 * extracted blobs. The blobs array must be freed using `destroy_blobs`
 * function.
 * A blob contains the label that was assigned to it, an array containing
 * the coordinates of the pixels making its external contour, the number
 * of internal contours (also called euler number), and the array of its
 * internal contours. Note that the internal contour array is not built
 * if `extract_internal` is set to 0.
 * 
 * If `extract_internal` is set to 1, each blob will stores their internal
 * contour points (if any). Otherwise, only the number of internal
 * contours will be set.
 * 
 * `find_blobs` returns 1 upon success or 0 if an error occured.
 * 
 * Note:
 * -----
 * The memory management is far from being optimal. 
 * This piece of code is not meant to be "production ready".
 * 
 */
#ifndef BLOB_INCLUDE_H
#define BLOB_INCLUDE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * Type for label buffer.
 * @warning The type must be signed.
 */
typedef int16_t label_t;

/**
 * Contour.
 * Array of contour pixel coordinates. A coordinate is stored as 2 int16_t.
 * The 1st one being X and the 2nd Y.
 */
typedef struct
{
    /** Number of points. **/
    int count;
    /** Number of allocated points. **/
    int capacity;
    /** Point array. **/
    int16_t *points;
} contour_t;

/**
 * Blob.
 */
typedef struct
{
    /** Label assigned to this blob. **/
    label_t label;
    /** External contour. **/
    contour_t  external;
    /** Internal contours (holes). **/
    contour_t *internal;
    /** Number of internal contour (also called euler number). **/
    int internal_count;
} blob_t;

/**
 * Compute connected components labels and contours.
 * @param [in]  roi_x   X coordinate of the upper left corner of the ROI.
 * @param [in]  roi_y   Y coordinate of the upper left corner of the ROI
 * @param [in]  roi_w   Width of the ROI.
 * @param [in]  roi_h   Height of the ROI.
 * @param [in]  in      Pointer to the input image buffer.
 * @param [in]  in_w    Width of the input image.
 * @param [in]  in_h    Height of the input image.
 * @param [out] label   Pointer to the label buffer.
 * @param [out] label_w Width of the label buffer (ROI clamped to image dimensions).
 * @param [out] label_h Height of the label buffer (ROI clamped to image dimensions).
 * @param [out] blobs   Array of extracted blobs.
 * @param [out] count   Number of extracted blobs.
 * @param [in]  extract_internal  Store internal contours in blob if set to 1.
 * @return 1 upon success or 0 if an error occured.
 */
int find_blobs(int16_t roi_x, int16_t roi_y, int16_t roi_w, int16_t roi_h,
               uint8_t *in, int16_t in_w, int16_t in_h, 
               label_t **label, int16_t *label_w, int16_t *label_h, 
               blob_t** blobs, int *count, int extract_internal);

/**
 * Destroy all blobs created by find_blobs.
 * @param [in] blobs Pointer to the array of blobs.
 * @param [in] count Number of blobs.
 */
void destroy_blobs(blob_t *blobs, int count);

#ifdef __cplusplus
}
#endif

#endif /* BLOB_INCLUDE_H */

/*--------------------------------------------------------------------
  -- Implementation start!
  --------------------------------------------------------------------*/
#ifdef BLOB_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

#if !defined(BLOB_ERROR)
#include <stdio.h>
#endif 

#if defined(BLOB_MALLOC) && defined(BLOB_REALLOC) && defined(BLOB_FREE)
// ok
#elif !defined(BLOB_MALLOC) && !defined(BLOB_REALLOC) && !defined(BLOB_FREE)
// ok
#else
#error "Must define all or none of BLOB_MALLOC, BLOB_REALLOC and BLOB_FREE"
#endif

#if !defined(BLOB_MALLOC)
#define BLOB_MALLOC(sz)         malloc(sz)
#define BLOB_REALLOC(p, new_sz) realloc(p, new_sz)
#define BLOB_FREE(p)            free(p)
#endif

#if !defined(BLOB_MEMSET)
#define BLOB_MEMSET(p, c, sz) memset(p, c, sz)
#endif

#if !defined(BLOB_ERROR)
#define BLOB_ERROR(format, ...) fprintf(stderr, format, ##__VA_ARGS__)
#endif

/* Add a point to contour */
static int contour_add_point(contour_t *contour, int16_t x, int16_t y)
{
    long int offset = contour->count * 2;
    if(contour->count == contour->capacity)
    {
        int newCapacity = contour->capacity ? (contour->capacity * 2) : 32;
        int16_t *tmp = (int16_t*)BLOB_REALLOC(contour->points, newCapacity * (2 * sizeof(int16_t)));
        if(NULL == tmp)
        {
            BLOB_ERROR("Out of memory");
            return 0;
        }
        contour->points = tmp;
        contour->capacity = newCapacity;
    }
    
    contour->points[offset  ] = x;
    contour->points[offset+1] = y;
    
    contour->count++;
    return 1;
}

/* Add a new blob */
static int blob_add(blob_t **b, int *count)
{
    blob_t *tmp = (blob_t*)BLOB_REALLOC(*b, (*count+1) * sizeof(blob_t));
    if(NULL == tmp)
    {
        BLOB_ERROR("Out of memory");
        return 0;
    }
    BLOB_MEMSET(&tmp[*count], 0, sizeof(blob_t));
    *b = tmp;
    *count += 1;
    return 1;
}

/* Add internal contour */
static int blob_add_internal(blob_t *b)
{
    contour_t *tmp = (contour_t*)BLOB_REALLOC(b->internal, (b->internal_count+1) * sizeof(contour_t));
    if(NULL == tmp)
    {
        BLOB_ERROR("Out of memory");
        return 0;
    }
    BLOB_MEMSET(&tmp[b->internal_count], 0, sizeof(contour_t));
    b->internal = tmp;
    b->internal_count++;
    return 1;
}

/* Destroy all blobs created by find_blobs. */
void destroy_blobs(blob_t *blobs, int count)
{
    int i;
    if(NULL == blobs)
    {
        return;
    }
    
    for(i=0; i<count; i++)
    {
        if(NULL != blobs[i].external.points)
        {
            BLOB_FREE(blobs[i].external.points);
        }

        if(NULL != blobs[i].internal)
        {
            int j;
            for(j=0; j<blobs[i].internal_count; j++)
            {
                if(NULL != blobs[i].internal[j].points)
                {
                    BLOB_FREE(blobs[i].internal[j].points);
                }
            }
            BLOB_FREE(blobs[i].internal);
        }
    }
    BLOB_FREE(blobs);
}

/* Extract blob contour (external or internal). */
static int contour_trace(uint8_t external, label_t current, int16_t x, int16_t y,
                         int16_t roi_x, int16_t roi_y, int16_t roi_w, int16_t roi_h,
                         uint8_t *in, int16_t line_stride, label_t *label, contour_t *contour)
{
    static int16_t dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
    static int16_t dy[8] = { 0, 1, 1, 1, 0,-1,-1,-1 };

    int i = external ? 7 : 3;
    int j;

    int16_t x0 = x;
    int16_t y0 = y;

    int16_t xx = -1;
    int16_t yy = -1;

    int done = 0;

    label[x0 + (roi_w * y0)] = current;

    while(!done)
    {
        if(NULL != contour)
        {
            if(0 == contour_add_point(contour, roi_x+x0, roi_y+y0))
            {
                return 0;
            }
        }

        /* Scan around current pixel in clockwise order. */
        for(j=0; j<8; j++, i=(i+1)&7)
        {
            const int16_t x1 = x0 + dx[i];
            const int16_t y1 = y0 + dy[i];
            const long int offset = x1 + (roi_w * y1);
            if((x1 < 0) || (x1 >= roi_w)) { continue; }
            if((y1 < 0) || (y1 >= roi_h)) { continue; }

            if(in[x1 + (line_stride * y1)])
            {
                label[offset] = current;

                if((xx < 0) && (yy < 0))
                {
                    xx = x1;
                    yy = y1;
                }
                else
                {
                    /* We are done if we crossed the first 2 contour points again. */
                    done =    ((x == x0) && (xx == x1)) 
                           && ((y == y0) && (yy == y1));
                }           
                x0 = x1;
                y0 = y1;
                break;
            }
            else
            {
                label[offset] = -1;
            }
        }
        /* Isolated point. */
        if(8 == j)
        {
            done = 1;
        }
        /* Compute next start position. */
        /* 1. Compute the neighbour index of the previous point. */
        int previous = (i+4) & 7;
        /* 2. Next search index is previous + 2 (mod 8). */
        i = (previous + 2) & 7;
    }
    
    return 1;
}

/* Compute connected components labels and contours. */
int find_blobs(int16_t roi_x, int16_t roi_y, int16_t roi_w, int16_t roi_h,
               uint8_t    *in,  int16_t     in_w, int16_t     in_h, 
               label_t **label, int16_t *label_w, int16_t *label_h, 
               blob_t** blobs, int *count, int extract_internal)
{
    uint8_t *ptr_in, *line_in, *roi_in;
    label_t *ptr_label;

    int16_t i, j;
    label_t current;

    /* sanity check. */
    if(   (NULL == label) || (NULL == label_w) || (NULL == label_h)
       || (NULL == blobs) || (NULL == count) )
    {
        BLOB_ERROR("One or more invalid arguments");
        return 0;
    }

    *blobs = NULL;
    *count = 0;

    /* adjust ROI */
    if((roi_x >= in_w) || (roi_y >= in_h))
    {
        /* nothing to do */
        return 1;
    }
    if(roi_x < 0) { roi_x = 0; }
    if(roi_y < 0) { roi_y = 0; }
    if((roi_x + roi_w) > in_w) { roi_w = in_w - roi_x; }
    if((roi_y + roi_h) > in_h) { roi_h = in_h - roi_h; }
    if((roi_w <= 0) || (roi_h <= 0))
    {
        /* nothing to do */
        return 1;
    }

    /* create label buffer */
    *label = (label_t*)malloc(roi_w * roi_h * sizeof(label_t));
    if(NULL == label)
    {
        BLOB_ERROR("Out of memory");
        return 0;
    }
    *label_w = roi_w;
    *label_h = roi_h;
    
    BLOB_MEMSET(*label, 0, roi_w * roi_h * sizeof(label_t));
    
    current = 1;

    roi_in = in + roi_x + (in_w * roi_y);
    line_in = roi_in;
    ptr_label = *label;
    
    for(j=0; j<roi_h; j++, line_in+=in_w)
    {
        for(i=0, ptr_in=line_in; i<roi_w; i++, ptr_in++, ptr_label++)
        {
            if(0 == *ptr_in) { continue; }
            
            const uint8_t above_in    = (j > 0) ? *(ptr_in - in_w) : 0;
            const uint8_t below_in    = (j < (roi_h-1)) ? *(ptr_in + in_w) : 0; 
            const label_t below_label = (j < (roi_h-1)) ? *(ptr_label + roi_w) : -1;
            /* 1. new external countour */
            if((0 == *ptr_label) && (0 == above_in))
            {
                /* add new blob */
                if( !blob_add(blobs, count) )
                {
                    return 0;
                }
                (*blobs+(*count-1))->label = current;
                /* trace external contour */
                contour_trace(1, current, i, j, roi_x, roi_y, roi_w, roi_h, roi_in, in_w, *label, &(*blobs+(*count-1))->external);
                ++current;
            }
            /* 2. new internal countour */
            else if((0 == below_in) && (0 == below_label))
            {
                label_t current_label = *ptr_label ? *ptr_label : *(ptr_label-1); // [todo] deserve a bit of explanation
                
                /* add a new internal contour to the corresponding blob. */
                blob_t *current_blob = *blobs + (current_label-1);
                contour_t *internal = NULL;
                if(extract_internal)
                {
                    if( !blob_add_internal(current_blob) )
                    {
                        return 0;
                    }
                    internal = current_blob->internal + current_blob->internal_count - 1;
                }
                else
                {
                    /* even if we don't want to store the internal contours,
                       we may want to know the number of holes. */
                    current_blob->internal_count++;
                }

                contour_trace(0, current_label, i, j, roi_x, roi_y, roi_w, roi_h, roi_in, in_w, *label, internal);
            }
            /* 3. internal element */
            else if(0 == *ptr_label)
            {
                *ptr_label = (i > 0) ? *(ptr_label - 1) : 0;
            }
        }
    }
    return 1;
}
#endif /* BLOB_IMPLEMENTATION */
