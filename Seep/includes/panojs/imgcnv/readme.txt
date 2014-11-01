BioImageConvertor v1.51

Author: Dima V. Fedorov <http://www.dimin.net/>

Arguments: [[-i | -o] FILE_NAME | -t FORMAT_NAME ]

Ex: imgcnv -i 1.jpg -o 2.tif -t TIFF

-c                    - additional channels input file name, multiple -c are allowed, in which case multiple channels will be added, -c image must have the same size

-create               - creates a new image with w-width, h-height, z-num z, t-num t, c - channels, d-bits per channel, ex: -create 100,100,1,1,3,8

-depth                - output depth (in bits) per channel, allowed values now are: 8,16,32,64, ex: -depth 8,D,U
  if followed by commma and [F|D|T|E] allowes to choose LUT method
    F - Linear full range
    D - Linear data range (default)
    T - Linear data range with tolerance ignoring very low values
    E - equalized  if followed by commma and U|S|F] the type of output image can be defined    U - Unsigned integer (with depths: 8,16,32)    S - Signed integer (with depths: 8,16,32)    F - Float (with depths: 32,64,80)

-display              - creates 3 channel image with preferred channel mapping

-fmt                  - print supported formats

-fmthtml              - print supported formats in HTML

-fmtxml               - print supported formats in XML

-fuse                 - Changes order and number of channels in the output additionally allowing combining channels
Channels separated by comma specifying output channel order (0 means empty channel)
multiple channels can be added using + sign, ex: -fuse 1+4,2+4+5,3

-fuse6                - Produces 3 channel image from up to 6 channels
Channels separated by comma in the following order: Red,Green,Blue,Yellow,Magenta,Cyan,Gray
(0 or empty value means empty channel), ex: -fuse6 1,2,3,4


-fuseGrey             - Produces 1 channel image averaging all input channels, uses RGB weights for 3 channel images and equal weights for all others, ex: -fuseGrey

-geometry             - redefines geometry for any incoming image with: z-num z, t-num t, ex: -geometry 5,1

-i                    - input file name, multiple -i are allowed, but in multiple case each will be interpreted as a 1 page image.

-ihst                 - read image histogram from the file and use for nhancement operations

-info                 - print image info

-loadomexml           - reads OME-XML from a file and writes if output format is OME-TIFF

-meta                 - print image's meta-data

-meta-custom          - print image's custom meta-data fields

-meta-parsed          - print image's parsed meta-data, excluding custom fields

-meta-raw             - print image's raw meta-data in one huge pile

-meta-tag             - prints contents of a requested tag, ex: -tag pixel_resolution

-multi                - creates a multi-paged image if possible (TIFF,AVI), enabled by default

-negative             - returns negative of input image

-no-overlap           - Skips frames that overlap with the previous non-overlapping frame, ex: -no-overlap 5
  argument defines maximum allowed overlap in , in the example it is 5


-norm                 - normalize input into 8 bits output

-o                    - output file name

-ohst                 - write image histogram to the file

-options              - specify encoder specific options, ex: -options "fps 15 bitrate 1000"

Encoder specific options
Video files AVI, SWF, MPEG, etc. encoder options:
  fps N - specify Frames per Second, where N is a float number, if empty or 0 uses default, ex: fps 29.9
  bitrate N - specify bitrate, where N is an integer number, if empty or 0 uses default, ex: bitrate 1000

JPEG encoder options:
  quality N - specify encoding quality 0-100, where 100 is best, ex: quality 90
  progressive no - disables progressive JPEG encoding
  progressive yes - enables progressive JPEG encoding (default)

TIFF encoder options:
  compression N - where N can be: none, packbits, lzw, fax, ex: compression none


-page                 - pages to extract, should be followed by page numbers separated by comma, ex: -page 1,2,5
  page enumeration starts at 1 and ends at number_of_pages
  page number can be a dash where dash will be substituted by a range of values, ex: -page 1,-,5  if dash is not followed by any number, maximum will be used, ex: '-page 1,-' means '-page 1,-,number_of_pages'
  if dash is a first caracter, 1 will be used, ex: '-page -,5' means '-page 1,-,5'

-project              - combines by MAX all inout frames into one

-projectmax           - combines by MAX all inout frames into one

-projectmin           - combines by MIN all inout frames into one

-raw                  - reads RAW image with w,h,c,d,p,e,t ex: -raw 100,100,3,8,10,0,uint8
  w-width, h-height, c - channels, d-bits per channel, p-pages
  e-endianness(0-little,1-big), if in doubt choose 0
  t-pixel type: int8|uint8|int16|uint16|int32|uint32|float|double, if in doubt choose uint8

-rawmeta              - print image's raw meta-data in one huge pile

-reg-points           - Defines quality for image alignment in number of starting points, ex: -reg-points 200
  Suggested range is in between 32 and 512, more points slow down the processing


-remap                - Changes order and number of channels in the output, channel numbers are separated by comma (0 means empty channel), ex: -remap 1,2,3

-resample             - Is the same as resize, the difference is resample is brute force and resize uses image pyramid for speed

-resize               - should be followed by: width and height of the new image, ex: -resize 640,480
  if one of the numbers is ommited or 0, it will be computed preserving aspect ratio, ex: -resize 640,,NN
  if followed by commma and [NN|BL|BC] allowes to choose interpolation method, ex: -resize 640,480,NN
  NN - Nearest neighbor (default)
  BL - Bilinear
  BC - Bicubic
  if followed by commma AR, the size will be used as maximum bounding box to resize preserving aspect ratio, ex: 640,640,NN,AR

-resize3d             - performs 3D interpolation on an input image, ex: -resize3d 640,480,16
  if one of the W/H numbers is ommited or 0, it will be computed preserving aspect ratio, ex: -resize3d 640,,16,NN
  if followed by commma and [NN|BL|BC] allowes to choose interpolation method, ex: -resize3d 640,480,16,BC
  NN - Nearest neighbor (default)
  TL - Trilinear
  TC - Tricubic
  if followed by commma AR, the size will be used as maximum bounding box to resize preserving aspect ratio, ex: 640,640,16,BC,AR

-resolution           - redefines resolution for any incoming image with: x,y,z,t where x,y,z are in microns and t in seconds  ex: -resolution 0.012,0.012,1,0

-roi                  - region of interest, should be followed by: x1,y1,x2,y2 that defines ROI rectangle, ex: -roi 10,10,100,100
  if x1 or y1 are ommited they will be set to 0, ex: -roi ,,100,100 means 0,0,100,100
  if x2 or y2 are ommited they will be set to image size, ex: -roi 10,10,, means 10,10,width-1,height-1

-rotate               - rotates the image by deg degrees, only accepted valueas now are: 90, -90 and 180

-sampleframes         - samples for reading every Nth frame (useful for videos), ex: -sampleframes 5

-single               - disables multi-page creation mode

-skip-frames-leading  - skip N initial frames of a sequence, ex: -skip-frames-leading 5

-skip-frames-trailing - skip N final frames of a sequence, ex: -skip-frames-trailing 5

-stretch              - stretch data to it's full range

-supported            - prints yes/no if the file can be decoded

-t                    - output format

-tile                 - tilte the image and store tiles in the output directory, ex: -tile 256
  argument defines the size of the tiles in pixels
  tiles will be created based on the outrput file name with inserted L, X, Y, where    L - is a resolution level, L=0 is native resolution, L=1 is 2x smaller, and so on    X and Y - are tile indices in X and Y, where the first tile is 0,0, second in X is: 1,0 and so on  ex: '-o my_file.jpg' will produce files: 'my_file_LLL_XXX_YYY.jpg'


-v                    - prints version

-verbose              - output information about the processing progress, ex: -verbose
  verbose allows argument that defines the amount of info, currently: 1 and 2
  where: 1 is the light info output, 2 is full output



---------------------------------------------------------------------------
Examples
---------------------------------------------------------------------------

*) To convert any given image into a set of tiffs:
  ./imgcnv -i in.img -o out.tif -t TIFF

All the pages will be sored as output format appending _XXXX (page number)
to the output file name.
  
*) To convert any given image into a set of jpegs you would have to make sure
it's normalized, later versions will normalize automatically given the output 
format cannot write 16bpp:  
  ./imgcnv -i in.img -o out.jpg -t JPEG -norm
  
*) To know all supported formats with their names:
  ./imgcnv -fmt
  
*) To extract textual metadata from the image:
  ./imgcnv -i in.img -meta
  
  After extracting meta-data the program will end, even if there's any output 
  file name set, this behaviour can be changed later.
  
*) To Region Of Interest (ROI) from image and save:
  ./imgcnv -i in.img -o out.tif -t TIFF -roi 10,10,100,100


