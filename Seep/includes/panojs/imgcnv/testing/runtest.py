#!/usr/bin/python 

# The following command line paratemetrs can be used in any combination:
# all     - execute all tests
# reading - execute reading tests
# writing - execute writing tests
# meta    - execute metadata reading tests
# video   - execute video reading and metadata tests

""" imgcnv testing framework
"""

__module__    = "imgcnv_testing"
__author__    = "Dmitry Fedorov"
__version__   = "1.6"
__revision__  = "$Rev$"
__date__      = "$Date$"
__copyright__ = "Center for BioImage Informatics, University California, Santa Barbara"

import sys
import os
from copy import deepcopy
from subprocess import Popen, call, PIPE
import time

IMGCNV='./imgcnv'
IMGCNVVER='1.50'

failed = 0
passed = 0
results = []

###############################################################
# misc
###############################################################
def version ():
    imgcnvver = Popen ([IMGCNV, '-v'],stdout=PIPE).communicate()[0]
    for line in imgcnvver.splitlines():
        if not line or line.startswith('Input'): return False
        return line.replace('\n', '')
 
def check_version ( needed ):
    inst = version()
    if not inst:            
        raise Exception('imgcnv was not found')
          
    inst_ver = inst.split('.')
    need_ver = needed.split('.')
    if int(inst_ver[0])<int(need_ver[0]) or int(inst_ver[1])<int(need_ver[1]): 
        raise Exception('Imgcnv needs update! Has: '+inst+' Needs: '+needed)   

def parse_imgcnv_info(s):
    d = {}
    for l in s.splitlines():
        k = l.split(': ', 1)
        if len(k)>1:
            d[k[0]] = k[1]
    return d
    
def print_failed(s, f='-'):
    print 'X FAILED %s'%(s)
    global failed
    global results
    failed += 1
    results.append( '%s: %s'%(f,s) )

def print_passed(s):
    print 'PASSED %s'%(s)
    global passed
    passed += 1    

def copy_keys(dict_in, keys_in):
    dict_out = {}
    for k in keys_in:
        if k in dict_in:
            dict_out[k] = dict_in[k]
    return dict_out    
    #return { ?zip(k,v)? for k in keys_in if k in dict_in }   

      
###############################################################
# info comparisons
###############################################################
      
class InfoComparator(object):
    '''Compares two info dictionaries''' 
    def compare(self, iv, tv):
        return False
    def fail(self, k, iv, tv):
        print_failed('%s failed comparison [%s] [%s]'%(k, iv, tv))

class InfoEquality(InfoComparator):
    def compare(self, iv, tv):
        return (iv==tv)
    def fail(self, k, iv, tv):
        print_failed('%s failed comparison %s = %s'%(k, iv, tv))

class InfoNumericLessEqual(InfoComparator):
    def compare(self, iv, tv):
        return (int(iv)<=int(tv))
    def fail(self, k, iv, tv):
        print_failed('%s failed comparison %s <= %s'%(k, iv, tv))


def compare_info(info, test, cc=InfoEquality() ):
    for tk in test:
        if tk not in info:
            print_failed('%s not found in info'%(tk))
            return False;
        if not cc.compare(info[tk], test[tk]):
            cc.fail( tk, info[tk], test[tk] )
            return False;  
    return True
               

###############################################################
# TESTS
###############################################################

def test_image_read( format, filename ):

    print    
    print '---------------------------------------'
    print '%s - %s'%(format, filename)
    print '---------------------------------------'    
  
  
    #------------------------------------------------------------------------
    # reading and converting into TIFF
    #------------------------------------------------------------------------      
    out_name = 'tests/_test_converting_%s.tif'%(filename)
    thumb_name = 'tests/_test_thumbnail_%s.jpg'%(filename)       
    out_fmt = 'tiff'
    filename = 'images/%s'%(filename)

    # test if file can be red
    command = [IMGCNV, '-i', filename, '-info']
    r = Popen (command, stdout=PIPE).communicate()[0]
    info_org = parse_imgcnv_info(r)
    
    if r is None or r.startswith('Input format is not supported') or len(info_org)<=0:
        print_failed('loading info', format)
        return        
    else:
        print_passed('loading info')

    # convert the file into TIFF   
    command = [IMGCNV, '-i', filename, '-o', out_name, '-t', out_fmt]
    r = Popen (command, stdout=PIPE).communicate()[0]    
        
    # get info from the converted file           
    command = [IMGCNV, '-i', out_name, '-info']
    r = Popen (command, stdout=PIPE).communicate()[0]
    info_cnv = parse_imgcnv_info(r)
    
    if r is None or r.startswith('Input format is not supported') or len(info_cnv)<=0:
        print_failed('loading converted info', format)
        return
    else:
        print_passed('loading converted info')
      
    # test if converted file has same info
    info_test = copy_keys(info_cnv, ('pages', 'channels', 'width', 'height', 'depth'))
    if compare_info(info_org, info_test)==True:
        print_passed('geometry')
    
    #------------------------------------------------------------------------
    # Writing thumbnail
    #------------------------------------------------------------------------    
    command = [IMGCNV, '-i', filename, '-o', thumb_name, '-t', 'jpeg', '-depth', '8,d', '-page', '1', '-display', '-resize', '128,128,BC,AR']
    r = Popen (command, stdout=PIPE).communicate()[0]    
    
    command = [IMGCNV, '-i', thumb_name, '-info']
    r = Popen (command, stdout=PIPE).communicate()[0]
    info_thb = parse_imgcnv_info(r)    
    
    if r is None or r.startswith('Input format is not supported') or len(info_thb)<=0:
        print_failed('loading thumbnail info', format)
        return
    else:
        print_passed('loading thumbnail info')    
        
    if compare_info( info_thb, {'pages':'1', 'channels':'3', 'depth':'8'} )==True:
        if compare_info(info_thb, {'width':'128', 'height':'128'}, InfoNumericLessEqual())==True:
            print_passed('thumbnail geometry')
    
    print    
    

def test_image_write( format, filename ):

    print    
    print '---------------------------------------'
    print '%s - %s'%(format, filename)
    print '---------------------------------------'    
  
    out_name = 'tests/_test_writing_%s.%s'%(filename, format)
    out_fmt = format
    filename = 'images/%s'%(filename) 

    # test if file can be red
    command = [IMGCNV, '-i', filename, '-info']
    r = Popen (command, stdout=PIPE).communicate()[0]
    info_org = parse_imgcnv_info(r)
    
    if r is None or r.startswith('Input format is not supported') or len(info_org)<=0:
        print_failed('loading input info', format)
        return        

    # convert the file into format   
    command = [IMGCNV, '-i', filename, '-o', out_name, '-t', out_fmt]
    r = Popen (command, stdout=PIPE).communicate()[0]    
        
    # get info from the converted file           
    command = [IMGCNV, '-i', out_name, '-info']
    r = Popen (command, stdout=PIPE).communicate()[0]
    info_cnv = parse_imgcnv_info(r)
    
    if r is None or r.startswith('Input format is not supported') or len(info_cnv)<=0:
        print_failed('loading written info', format)
        return
    else:
        print_passed('loading written info')
      
    # test if converted file has same info
    info_test = copy_keys(info_cnv, ('pages', 'channels', 'width', 'height', 'depth'))
    if compare_info(info_org, info_test)==True:
        print_passed('written geometry')
    
    print       


def test_video_write( format, filename ):

    print    
    print '---------------------------------------'
    print '%s - %s'%(format, filename)
    print '---------------------------------------'    
  
    out_name = 'tests/_test_writing_%s.%s'%(filename, format)
    out_fmt = format
    filename = 'images/%s'%(filename) 

    # test if file can be red
    command = [IMGCNV, '-i', filename, '-info']
    r = Popen (command, stdout=PIPE).communicate()[0]
    info_org = parse_imgcnv_info(r)
    
    if r is None or r.startswith('Input format is not supported') or len(info_org)<=0:
        print_failed('loading input info', format)
        return        

    # convert the file into format   
    command = [IMGCNV, '-i', filename, '-o', out_name, '-t', out_fmt]
    r = Popen (command, stdout=PIPE).communicate()[0]    
        
    # get info from the converted file           
    command = [IMGCNV, '-i', out_name, '-info']
    r = Popen (command, stdout=PIPE).communicate()[0]
    info_cnv = parse_imgcnv_info(r)
    
    if r is None or r.startswith('Input format is not supported') or len(info_cnv)<=0:
        print_failed('loading written info', format)
        return
    else:
        print_passed('loading written info')
      
    # test if converted file has same info
    info_test = copy_keys(info_cnv, ('width', 'height'))
    if compare_info(info_org, info_test)==True:
        print_passed('written geometry')
    
    print   


def test_image_metadata( format, filename, meta_test, meta_test_cnv=None  ):

    print   
    print '---------------------------------------'
    print '%s - %s'%(format, filename)
    print '---------------------------------------'    
  
    out_name = 'tests/_test_metadata_%s.ome.tif'%(filename)
    out_fmt = 'ome-tiff'
    filename = 'images/%s'%(filename)    

    # test if file can be red
    command = [IMGCNV, '-i', filename, '-meta-parsed']
    r = Popen (command, stdout=PIPE).communicate()[0]
    meta_org = parse_imgcnv_info(r)
    
    if r is None or r.startswith('Input format is not supported') or len(meta_org)<=0:
        print_failed('loading metadata', format)
        return        

    #print str(meta_org)

    # test if converted file has same info
    if compare_info(meta_org, meta_test)==True:
        print_passed('reading metadata')

    # convert the file into format   
    command = [IMGCNV, '-i', filename, '-o', out_name, '-t', out_fmt]
    r = Popen (command, stdout=PIPE).communicate()[0] 
        
    # get info from the converted file           
    command = [IMGCNV, '-i', out_name, '-meta-parsed']
    r = Popen (command, stdout=PIPE).communicate()[0]
    meta_cnv = parse_imgcnv_info(r)
    
    if r is None or r.startswith('Input format is not supported') or len(meta_cnv)<=0:
        print_failed('loading written metadata', format)
        return
    else:
        print_passed('loading written metadata')

    #print str(meta_cnv)
   
    if meta_test_cnv is None: meta_test_cnv=meta_test
    if compare_info(meta_cnv, meta_test_cnv)==True:
        print_passed('writing metadata')
            
    print 
    
def test_metadata_read( format, filename, meta_test  ):

    print   
    print '---------------------------------------'
    print '%s - %s'%(format, filename)
    print '---------------------------------------'    
  
    filename = 'images/%s'%(filename)    

    # test if file can be red
    command = [IMGCNV, '-i', filename, '-meta']
    r = Popen (command, stdout=PIPE).communicate()[0]
    meta_org = parse_imgcnv_info(r)
    
    if r is None or r.startswith('Input format is not supported') or len(meta_org)<=0:
        print_failed('reading meta-data', format)
        return        

    # test if converted file has same info
    if compare_info(meta_org, meta_test)==True:
        print_passed('reading meta-data')
            
    print     
    
def test_image_video( format, filename, meta_test  ):

    print   
    print '---------------------------------------'
    print '%s - %s'%(format, filename)
    print '---------------------------------------'    
  
    out_name = 'tests/_test_metadata_%s.ome.tif'%(filename)
    out_fmt = 'ome-tiff'
    filename = 'images/%s'%(filename)    

    # test if file can be red
    command = [IMGCNV, '-i', filename, '-meta-parsed']
    r = Popen (command, stdout=PIPE).communicate()[0]
    meta_org = parse_imgcnv_info(r)
    
    if r is None or r.startswith('Input format is not supported') or len(meta_org)<=0:
        print_failed('reading video', format)
        return        

    #print str(meta_org)

    # test if converted file has same info
    if compare_info(meta_org, meta_test)==True:
        print_passed('reading video info')
            
    print 
        

###############################################################
# run tests
###############################################################

check_version ( IMGCNVVER )

try:
    os.mkdir('tests')
except:
    pass

mode = sys.argv
if len(mode) <= 1: mode.append('all')

start = time.time()

if 'all' in mode or 'reading' in mode:
    print
    print
    print '***************************************************'
    print 'Reading formats, converting and creating thumbnails'
    print '***************************************************'  
    
    test_image_read( "BMP", "107_07661.bmp" )
    test_image_read( "JPEG", "A01.jpg" )
    test_image_read( "PNG", "flowers_24bit_nointr.png" )
    test_image_read( "BIORAD-PIC", "MZ2.PIC" )
    test_image_read( "FLUOVIEW", "161pkcvampz1Live2-17-2004_11-57-21_AM.tif" )
    test_image_read( "Zeiss LSM", "combinedsubtractions.lsm" )
    test_image_read( "Zeiss LSM 1 ch", "HEK293_Triple_Dish1_set_9.lsm" )    
    test_image_read( "OME-TIFF", "wta.ome.tif" )
    test_image_read( "TIFF float", "autocorrelation.tif" )
    test_image_read( "STK", "K560-tax-6-7-7-1.stk" )
    test_image_read( "STK", "MDD2-7.stk" )
    test_image_read( "STK", "sxn3_w1RGB-488nm_s1.stk" )
    test_image_read( "STK", "7.5tax10.stk" )
    test_image_read( "OIB", "test z1024 Image0004.oib" )
    test_image_read( "OIB", "MB_10X_20100303.oib" )
    test_image_read( "NANOSCOPE", "AXONEME.002" )
    test_image_read( "IBW", "tubule20000.ibw" )
    test_image_read( "PSIA", "040130Topography001.tif" )
    test_image_read( "BigTIFF", "test16bit.btf" )
    #test_image_read( "OME-BigTIFF", "bigtiff.ome.btf" )
    test_image_read( "QuickTime", "3Dstack.tif.3D.mov" )
    test_image_read( "AVI", "radiolaria.avi" )
    test_image_read( "ZVI", "23D3HA-cy3 psd-gfp-488 Homer-647 DIV 14 - 3.zvi" )
    test_image_read( "ZVI", "0022.zvi" )


if 'all' in mode or 'writing' in mode:
    print
    print
    print '***************************************************'
    print 'Writing formats'
    print '***************************************************'  
    
    test_image_write( "JPEG", "flowers_24bit_nointr.png" )
    test_image_write( "PNG", "flowers_24bit_nointr.png" )
    test_image_write( "BMP", "flowers_24bit_nointr.png" )
    test_image_write( "TIFF", "flowers_24bit_nointr.png" )
    test_image_write( "OME-TIFF", "161pkcvampz1Live2-17-2004_11-57-21_AM.tif" )
    test_image_write( "BigTIFF", "161pkcvampz1Live2-17-2004_11-57-21_AM.tif" )
    #test_image_write( "OME-BigTIFF", "161pkcvampz1Live2-17-2004_11-57-21_AM.tif" )
    
    test_video_write( "AVI", "161pkcvampz1Live2-17-2004_11-57-21_AM.tif" )
    test_video_write( "QuickTime", "161pkcvampz1Live2-17-2004_11-57-21_AM.tif" )
    test_video_write( "MPEG4", "161pkcvampz1Live2-17-2004_11-57-21_AM.tif" )
    test_video_write( "WMV", "161pkcvampz1Live2-17-2004_11-57-21_AM.tif" )
    test_video_write( "OGG", "161pkcvampz1Live2-17-2004_11-57-21_AM.tif" )

#sys.exit (0)

if 'all' in mode or 'meta' in mode:
    print
    print
    print '***************************************************'
    print 'Reading and converting metadata'
    print '***************************************************'  
    
    meta_test = {}
    meta_test['image_num_z'] = '6'
    meta_test['image_num_t'] = '1'
    meta_test['pixel_resolution_x'] = '0.192406'
    meta_test['pixel_resolution_y'] = '0.192406'
    meta_test['pixel_resolution_z'] = '1.185000'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    meta_test['pixel_resolution_unit_z'] = 'microns'
    test_image_metadata( "PIC", "MZ2.PIC", meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '13'
    meta_test['image_num_t'] = '1'
    meta_test['pixel_resolution_x'] = '0.207160'
    meta_test['pixel_resolution_y'] = '0.207160'
    meta_test['pixel_resolution_z'] = '1.000000'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    meta_test['pixel_resolution_unit_z'] = 'microns'
    meta_test['channel_0_name'] = 'FITC'
    meta_test['channel_1_name'] = 'Cy3'
    meta_test_full = deepcopy(meta_test)
    meta_test_full['objective'] = 'UPLAPO 40XO'
    meta_test_full['magnification'] = '40X'
    test_image_metadata( "TIFF Fluoview", "161pkcvampz1Live2-17-2004_11-57-21_AM.tif", meta_test_full, meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '30'
    meta_test['image_num_t'] = '1'
    meta_test['pixel_resolution_x'] = '0.138661'
    meta_test['pixel_resolution_y'] = '0.138661'
    meta_test['pixel_resolution_z'] = '1.000000'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    meta_test['pixel_resolution_unit_z'] = 'microns'
    meta_test_full = deepcopy(meta_test)
    meta_test_full['objective'] = 'Achroplan 63x/0.95 W'
    test_image_metadata( "TIFF Zeiss LSM", "combinedsubtractions.lsm", meta_test_full, meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '152'
    meta_test['image_num_t'] = '1'
    meta_test['pixel_resolution_x'] = '0.124000'
    meta_test['pixel_resolution_y'] = '0.124000'
    meta_test['pixel_resolution_z'] = '0.350000'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    meta_test['pixel_resolution_unit_z'] = 'microns'
    meta_test['channel_0_name'] = 'Alexa Fluor 488 - 488nm'
    meta_test['channel_1_name'] = 'Alexa Fluor 546 - 543nm'
    meta_test['channel_2_name'] = 'DRAQ5 - 633nm'
    meta_test_full = deepcopy(meta_test)
    meta_test_full['objective'] = 'UPLFLN    40X O  NA:1.30'
    meta_test_full['magnification'] = '40X'
    test_image_metadata( "OME-TIFF", "wta.ome.tif", meta_test_full, meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '31'
    meta_test['pixel_resolution_t'] = '4.000000'
    test_image_metadata( "STK", "K560-tax-6-7-7-1.stk", meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '105'
    meta_test['image_num_t'] = '1'
    meta_test['pixel_resolution_x'] = '0.430000'
    meta_test['pixel_resolution_y'] = '0.430000'
    meta_test['pixel_resolution_z'] = '0.488000'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    meta_test['pixel_resolution_unit_z'] = 'microns'
    meta_test_full = deepcopy(meta_test)
    meta_test_full['stage_distance_z'] = '0.488000'
    meta_test_full['stage_position_x'] = '3712.200000'
    meta_test_full['stage_position_y'] = '-2970.340000'
    meta_test_full['stage_position_z'] = '25.252000'
    test_image_metadata( "STK", "sxn3_w1RGB-488nm_s1.stk", meta_test_full, meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '16'
    meta_test['image_num_t'] = '1'
    meta_test['pixel_resolution_x'] = '2.483410'
    meta_test['pixel_resolution_y'] = '2.480193'
    meta_test['pixel_resolution_z'] = '0.937500'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    meta_test['pixel_resolution_unit_z'] = 'microns'
    meta_test['channel_0_name'] = 'Alexa Fluor 405'
    meta_test['channel_1_name'] = 'Cy3'
    test_image_metadata( "OIB", "MB_10X_20100303.oib", meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '1'
    meta_test['pixel_resolution_x'] = '0.177539'
    meta_test['pixel_resolution_y'] = '0.177539'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    test_image_metadata( "PSIA", "040130Topography001.tif", meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '1'
    #meta_test['image_num_p'] = '2'
    meta_test['pixel_resolution_x'] = '0.058594'
    meta_test['pixel_resolution_y'] = '0.058594'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    test_image_metadata( "NANOSCOPE", "AXONEME.002", meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '1'
    #meta_test['image_num_p'] = '3'
    test_image_metadata( "IBW", "tubule20000.ibw", meta_test )
    
    ##meta_test = {}
    ##meta_test['image_num_z'] = '13'
    ##meta_test['image_num_t'] = '1'
    ##meta_test['pixel_resolution_x'] = '0.207160'
    ##meta_test['pixel_resolution_y'] = '0.207160'
    ##meta_test['pixel_resolution_z'] = '1.000000'
    ##meta_test['pixel_resolution_unit_x'] = 'microns'
    ##meta_test['pixel_resolution_unit_y'] = 'microns'
    ##meta_test['pixel_resolution_unit_z'] = 'microns'
    ##meta_test['channel_0_name'] = 'FITC'
    ##meta_test['channel_1_name'] = 'Cy3'
    ##test_image_metadata( "OME-BigTIFF", "bigtiff.ome.btf", meta_test )

if 'all' in mode or 'readmeta' in mode:
    print
    print
    print '***************************************************'
    print 'Reading and parsing metadata'
    print '***************************************************'  

    # testing extracting GPS tags from EXIF
    meta_test = {}
    meta_test['Exif/GPSInfo/GPSLatitude']    = '34deg 26.27000\''
    meta_test['Exif/GPSInfo/GPSLatitudeRef'] = 'North'
    test_metadata_read( "JPEG EXIF", "IMG_0488.JPG", meta_test )

    # testing extracting IPTC tags
    meta_test = {}
    meta_test['Iptc/Application2/City']    = 'Santa Barbara'
    meta_test['Iptc/Application2/ProvinceState'] = 'CA'
    test_metadata_read( "JPEG IPTC", "IMG_0184.JPG", meta_test )

    # reading metadata from OIB v 2.0.0.0 
    meta_test = {}
    meta_test['date_time'] = '2010-06-04 08:26:06'   
    meta_test['pixel_resolution_x'] = '0.206798'
    meta_test['pixel_resolution_y'] = '0.206798'
    meta_test['pixel_resolution_z'] = '0.428571'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    meta_test['pixel_resolution_unit_z'] = 'microns'
    meta_test['channel_0_name'] = 'Cy2'
    meta_test['channel_1_name'] = 'Cy3'
    meta_test['channel_2_name'] = 'Cy5'  
    meta_test['display_channel_blue'] = '2'  
    meta_test['display_channel_cyan'] = '-1'  
    meta_test['display_channel_gray'] = '-1'  
    meta_test['display_channel_green'] = '0'  
    meta_test['display_channel_magenta'] = '-1'  
    meta_test['display_channel_red'] = '1'  
    meta_test['display_channel_yellow'] = '-1'  
    test_metadata_read( "OIB.2", "CSR 4 mo COX g M cone r PNA b z1.oib", meta_test )
    
    # reading metadata from large OIB 
    meta_test = {}
    meta_test['image_num_x'] = '1024'   
    meta_test['image_num_y'] = '1024'   
    meta_test['image_num_c'] = '1'
    meta_test['image_num_t'] = '1'
    meta_test['image_num_z'] = '528'
    meta_test['pixel_resolution_x'] = '1.240787'
    meta_test['pixel_resolution_y'] = '1.240787'
    meta_test['pixel_resolution_z'] = '1.237652'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    meta_test['pixel_resolution_unit_z'] = 'microns'
    meta_test['date_time'] = '2012-01-25 15:35:02'   
    meta_test['channel_0_name'] = 'None'
    test_metadata_read( "OIB.Large", "112811B_5.oib", meta_test )    
    
    # reading metadata from OIB with strange Z planes and only actual one Z image 
    meta_test = {}
    meta_test['image_num_c'] = '4'
    meta_test['image_num_p'] = '1'
    meta_test['image_num_z'] = '1'    
    meta_test['image_num_t'] = '1'        
    meta_test['image_num_x'] = '640'   
    meta_test['image_num_y'] = '640'           
    meta_test['image_pixel_depth'] = '16'      
    meta_test['date_time'] = '2010-11-19 14:32:33' 
    meta_test['pixel_resolution_x'] = '0.496223'
    meta_test['pixel_resolution_y'] = '0.496223'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    meta_test['channel_0_name'] = 'Alexa Fluor 405'
    meta_test['channel_1_name'] = 'Cy2'
    meta_test['channel_2_name'] = 'Cy3'
    meta_test['channel_3_name'] = 'Cy5'        
    meta_test['display_channel_red']     = '2'        
    meta_test['display_channel_green']   = '1'        
    meta_test['display_channel_blue']    = '3'        
    meta_test['display_channel_cyan']    = '-1'        
    meta_test['display_channel_magenta'] = '-1'        
    meta_test['display_channel_yellow']  = '-1'        
    meta_test['display_channel_gray']    = '0'        
    test_metadata_read( "OIB 2.0 One plane", "Retina 4 top.oib", meta_test )        
    
    
    # reading metadata from Zeiss ZVI
    meta_test = {}
    meta_test['date_time'] = '2006-06-22 08:27:13'   
    meta_test['pixel_resolution_x'] = '0.157153'
    meta_test['pixel_resolution_y'] = '0.157153'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    meta_test['channel_0_name'] = 'Cy5'
    meta_test['channel_1_name'] = 'TRITC'
    meta_test['channel_2_name'] = 'FITC'  
    meta_test['channel_3_name'] = 'DAPI'      
    meta_test['magnification'] = '40.000000'
    meta_test['objective'] = 'Plan Neofluar 40x/1.30 Oil Ph3 (DIC III) (440451)'  
    meta_test['objective_numerical_aperture'] = '1.300000'    
    test_metadata_read( "Zeiss ZVI", "23D3HA-cy3 psd-gfp-488 Homer-647 DIV 14 - 3.zvi", meta_test )    

    # reading metadata from Zeiss ZVI
    meta_test = {}
    meta_test['image_num_c'] = '2'
    meta_test['image_num_p'] = '14'
    meta_test['image_num_t'] = '1'
    meta_test['image_num_z'] = '14'
    meta_test['image_pixel_depth'] = '16'            
    meta_test['image_num_x'] = '1388'   
    meta_test['image_num_y'] = '1040'           
    meta_test['date_time'] = '2010-01-06 03:53:37'   
    meta_test['pixel_resolution_x'] = '0.102381'
    meta_test['pixel_resolution_y'] = '0.102381'
    meta_test['pixel_resolution_z'] = '0.320000'    
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    meta_test['pixel_resolution_unit_z'] = 'microns'    
    meta_test['channel_0_name'] = 'DsRed'
    meta_test['channel_1_name'] = 'eGFP'
    meta_test['display_channel_red'] = '-1'
    meta_test['display_channel_green'] = '0'
    meta_test['display_channel_blue'] = '1'
    meta_test['display_channel_yellow'] = '-1'
    meta_test['display_channel_magenta'] = '-1'
    meta_test['display_channel_cyan'] = '-1'
    meta_test['display_channel_gray'] = '-1'                
    meta_test['magnification'] = '63.000000'
    meta_test['objective'] = 'C-Apochromat 63x/1.20 W Korr UV VIS IR'  
    meta_test['objective_numerical_aperture'] = '1.200000'    
    test_metadata_read( "Zeiss ZVI", "0022.zvi", meta_test )    


    # reading metadata from Andor
    meta_test = {}
    meta_test['image_num_c'] = '1'
    meta_test['image_num_p'] = '12'
    meta_test['image_num_x'] = '512'   
    meta_test['image_num_y'] = '512'           
    meta_test['date_time'] = '2010-06-08 10:10:36'   
    meta_test['pixel_resolution_x'] = '1.000000'
    meta_test['pixel_resolution_y'] = '1.000000'
    meta_test['pixel_resolution_unit_x'] = 'microns'
    meta_test['pixel_resolution_unit_y'] = 'microns'
    meta_test['stage_position/0/x'] = '923.497006'
    meta_test['stage_position/0/y'] = '-1660.502994'
    meta_test['stage_position/0/z'] = '49.990000'    
    test_metadata_read( "Andor", "MF Mon 2x2.tif", meta_test )    




if 'all' in mode or 'video' in mode:
    print
    print
    print '***************************************************'
    print 'Reading video'
    print '***************************************************'  
    
    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '101'
    meta_test['image_num_p'] = '101'
    meta_test['image_num_x'] = '720'
    meta_test['image_num_y'] = '480'
    meta_test['video_frames_per_second'] = '29.970000'
    meta_test['video_codec_name'] = 'cinepak'
    test_image_video( "AVI CINEPAK", "122906_3Ax(inverted).avi", meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '37'
    meta_test['image_num_p'] = '37'
    meta_test['image_num_x'] = '640'
    meta_test['image_num_y'] = '480'
    meta_test['video_frames_per_second'] = '20.000000'
    meta_test['video_codec_name'] = 'cinepak'
    test_image_video( "AVI CINEPAK", "241aligned.avi", meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '199'
    meta_test['image_num_p'] = '199'
    meta_test['image_num_x'] = '826'
    meta_test['image_num_y'] = '728'
    meta_test['video_frames_per_second'] = '30.000000'
    meta_test['video_codec_name'] = 'mpeg4'
    test_image_video( "QuickTime MPEG4", "3Dstack.tif.3D.mov", meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '73'
    meta_test['image_num_p'] = '73'
    meta_test['image_num_x'] = '1024'
    meta_test['image_num_y'] = '947'
    meta_test['video_frames_per_second'] = '30.303030'
    meta_test['video_codec_name'] = 'rawvideo'
    test_image_video( "AVI RAW", "B4nf.RS.RE.z1.5.15.06.3DAnimation.avi", meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '2879'
    meta_test['image_num_p'] = '2879'
    meta_test['image_num_x'] = '352'
    meta_test['image_num_y'] = '240'
    meta_test['video_frames_per_second'] = '29.970030'
    meta_test['video_codec_name'] = 'mpeg1video'
    test_image_video( "MPEG", "EleanorRigby.mpg", meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '301'
    meta_test['image_num_p'] = '301'
    meta_test['image_num_x'] = '884'
    meta_test['image_num_y'] = '845'
    meta_test['video_frames_per_second'] = '29.970030'
    meta_test['video_codec_name'] = 'mpeg4'
    test_image_video( "QuickTime MPEG4", "Muller cell z4.oib.3D.mov", meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '9'
    meta_test['image_num_p'] = '9'
    meta_test['image_num_x'] = '316'
    meta_test['image_num_y'] = '400'
    meta_test['video_frames_per_second'] = '9.000009'
    meta_test['video_codec_name'] = 'rawvideo'
    test_image_video( "AVI RAW", "radiolaria.avi", meta_test )
    
    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '2770'
    meta_test['image_num_p'] = '2770'
    meta_test['image_num_x'] = '1440'
    meta_test['image_num_y'] = '1080'
    meta_test['video_frames_per_second'] = '23.976024'
    meta_test['video_codec_name'] = 'wmv3'
    test_image_video( "WMV", "Step_into_Liquid_1080.wmv", meta_test )

    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '199'
    meta_test['image_num_p'] = '199'
    meta_test['image_num_x'] = '826'
    meta_test['image_num_y'] = '728'
    meta_test['video_frames_per_second'] = '29.970030'
    meta_test['video_codec_name'] = 'theora'
    test_image_video( "OGG", "test.ogv", meta_test )

    meta_test = {}
    meta_test['image_num_z'] = '1'
    meta_test['image_num_t'] = '36776'
    meta_test['image_num_p'] = '36776'
    meta_test['image_num_x'] = '1440'
    meta_test['image_num_y'] = '1080'
    meta_test['video_frames_per_second'] = '29.970030'
    meta_test['video_codec_name'] = 'mpeg2video'
    test_image_video( "MPEG2 TS", "B01C0201.M2T", meta_test )


end = time.time()
elapsed= end - start

# print summary
print '\n\nFollowing tests took %s seconds:\n'%(elapsed)

if passed>0 and failed==0:
    print 'Passed all %d tests. Congrats!!!\n'%(passed)
    
if passed>0 and failed>0:
    print 'Passed: %d\n'%(passed)

if failed>0:
    print 'Failed: %d\n'%(failed)
    for s in results:
      print '  > %s'%(s)
