#!/usr/bin/env python3

"""
Converts a directory of .wav files to a C header file of uint8_t PWM audio data

Usage: wav2pwm.py <output_filename.h>

Notes:
    - The following files are expected to be present in the same directory
      as this script
            poweron.wav     Ignition sound
            poweroff.wav    Deactivation sound
            hum.wav         Idle sound
      as well as an indeterminant number of
            swing0.wav, swing1.wav, ...
            clash0.wav, clash1.wav, ...
      files for swing and clash sounds
      
    - Please supply only one poweron, poweroff, and hum sound!

    - The output file is a C header file containing the following:
            - A TUNE_POWERON_LEN, TUNE_POWEROFF_LEN, ... constant
            - A TUNE_POWERON_DATA, TUNE_POWEROFF_DATA, ... array of uint8_t
            - Swing sounds are accessible as TUNES_SWING_DATA[0], 
              TUNES_SWING_DATA[1], ..., and similarly with clash sounds
"""

import soundfile as sf
import samplerate
import argparse 
import os

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("output", help="Output .h file")

outfile = parser.parse_args().output


converter = 'sinc_best'  # or 'sinc_fastest', ...
desired_sample_rate = 44100.0



# Converts one audio file and returns its length
def audio_convert(name_base, wf, of):
    # Print some helpful identifying information in the header file
    of.write("// "+wf+"\n")
    data_in, datasamplerate = sf.read(wf)

    # If data is stereo, take only the first channel
    if len(data_in.shape)>1:
        data_in = data_in[:,0]
    ratio = desired_sample_rate/datasamplerate
    data_out = samplerate.resample(data_in, ratio, converter)
    maxValue = max(data_out)
    minValue = min(data_out)
    vrange = (maxValue - minValue) 

    of.write("#define TUNE_" + name_base + "_LEN "+str(len(data_out))+" \r\n\r\n")
    of.write("const uint8_t __in_flash() TUNE_" + name_base + "_DATA[] = {\r\n    ")

    maxitemsperline = 16
    itemsonline = maxitemsperline
    firstvalue = 0
    lastvalue = 0
    count = 0
    for v in data_out:
        # scale v to between 0 and 1
        isin = (v-minValue)/vrange   
        v =  int((isin * 255))
        vstr = str(v)
        if (firstvalue==0):
            firstvalue= v
        lastvalue = v
        of.write(vstr)
        itemsonline-=1
        if (count == len(data_out) - 1):
            of.write("\r\n")
        elif (itemsonline>0):
            of.write(',')
        else:
            itemsonline = maxitemsperline
            of.write(',\r\n    ')
        count += 1
            
    # keep track of first and last values to avoid
    # blip when the loop restarts.. make the end value
    # the average of the first and last. 
    #end_value = int( (firstvalue + lastvalue) / 2)
    #of.write(str(end_value)+'    \r\n};')
    
    of.write('};\r\n\n')
    
    return len(data_out)



with open(outfile, 'w') as of:
    of.write("/**\n")
    of.write(" * @file    "+outfile+"\n")
    of.write(" * @brief   <DESCRIPTION>\n")
    of.write(" */\n\n\n")
    of.write("#include <pico/platform.h>\n\n\n");

    # Get all files in the current directory
    files = os.listdir(os.getcwd())

    # Get all .wav files only
    wav_files = [f for f in files if f.endswith(".wav")]

    total_size = 0;
    
    # First, process the power on sound
    for wf in wav_files:
        if wf.startswith("poweron"):
            total_size += audio_convert("POWERON", wf, of)
            break

    # Next, process the power off sound
    for wf in wav_files:
        if wf.startswith("poweroff"):
            total_size += audio_convert("POWEROFF", wf, of)
            break
    
    # Next, process the hum sound
    for wf in wav_files:
        if wf.startswith("hum"):
            total_size += audio_convert("HUM", wf, of)
            break

    # Next, process any swing sounds
    swing_file_count = 0
    for wf in wav_files:
        if wf.startswith("swing"):
            total_size += audio_convert("SWING" + str(swing_file_count), wf, of)
            swing_file_count += 1

    # Finally, process any clash sounds
    clash_file_count = 0
    for wf in wav_files:
        if wf.startswith("clash"):
            total_size += audio_convert("CLASH" + str(clash_file_count), wf, of)
            clash_file_count += 1

    # For convenience, defines for the number of swing and clash sounds
    of.write("#define TUNES_SWING_COUNT "+str(swing_file_count)+"\r\n")
    of.write("#define TUNES_CLASH_COUNT "+str(clash_file_count)+"\r\n\r\n")
    
    # Make C arrays to be able to access swing and clash sounds more easily
    of.write("const uint8_t *TUNES_SWING_DATA[] = {\r\n")
    for i in range(swing_file_count):
        of.write("    TUNE_SWING"+str(i)+"_DATA")
        if i < swing_file_count-1: of.write(",")
        of.write("\r\n")
    of.write("};\r\n\r\n")
    of.write("uint32_t TUNES_SWING_LENS[] = {\r\n")
    for i in range(swing_file_count):
        of.write("    TUNE_SWING"+str(i)+"_LEN")
        if i < swing_file_count-1: of.write(",")
        of.write("\r\n")
    of.write("};\r\n\r\n")

    of.write("const uint8_t *TUNES_CLASH_DATA[] = {\r\n")
    for i in range(clash_file_count):
        of.write("    TUNE_CLASH"+str(i)+"_DATA")
        if i < clash_file_count-1: of.write(",")
        of.write("\r\n")
    of.write("};\r\n\r\n")
    of.write("uint32_t TUNES_CLASH_LENS[] = {\r\n")
    for i in range(clash_file_count):
        of.write("    TUNE_CLASH"+str(i)+"_LEN")
        if i < clash_file_count-1: of.write(",")
        of.write("\r\n")
    of.write("};\r\n\r\n")
    
    of.write("// Total size: " + str(total_size) + "\r\n\r\n");

    of.close()

'''
soundfile = parser.parse_args().input

data_in, datasamplerate = sf.read(soundfile)
# If data is stereo, take only the first channel
if len(data_in.shape)>1:
    data_in = data_in[:,0]

converter = 'sinc_best'  # or 'sinc_fastest', ...
desired_sample_rate = 22000.0
ratio = desired_sample_rate/datasamplerate
data_out = samplerate.resample(data_in, ratio, converter)
#print(data_out)
maxValue = max(data_out)
minValue = min(data_out)
#print("length", len(data_out))
#print("max value", max(data_out))
#print("min value", min(data_out))
vrange = (maxValue - minValue) 
#print("value range", vrange)

with open(outfile, 'w') as f:
    f.write("/*    File "+soundfile+ "\r\n *    Sample rate "+str(int(desired_sample_rate)) +" Hz\r\n */\r\n")
    f.write("#define WAV_DATA_LENGTH "+str(len(data_out))+" \r\n\r\n")
    f.write("uint8_t WAV_DATA[] = {\r\n    ")
    maxitemsperline = 16
    itemsonline = maxitemsperline
    firstvalue = 0
    lastvalue = 0
    for v in data_out:
        # scale v to between 0 and 1
        isin = (v-minValue)/vrange   
        v =  int((isin * 255))
        vstr = str(v)
        if (firstvalue==0):
            firstvalue= v
        lastvalue = v
        f.write(vstr)
        itemsonline-=1
        if (itemsonline>0):
            f.write(',')
        else:
            itemsonline = maxitemsperline
            f.write(',\r\n    ')
            
    # keep track of first and last values to avoid
    # blip when the loop restarts.. make the end value
    # the average of the first and last. 
    end_value = int( (firstvalue + lastvalue) / 2)
    f.write(str(end_value)+'    \r\n};')

f.close()
'''

