# mushraTests

mushraTests is a powerful tool to do MUSHRA (ITU-R BS.1534-3) tests with multichannel sound sources. It has been developed for the *Comparison of the spatial properties of Ambisonics versus the Spatial Wavelet Format (SWF)* master thesis by Rubén Eguinoa Cabrito.

You can do tests with a maximun of 6 stimuli plus the reference and the anchor.

## How to design a test

To design your own test, you need to fill the *testDescription.json* file. This file will contain all the information needed to run correctly the test:
 * Desc: the description of the test.
 * Groups: the number of groups of your test.
 * StimulisPerGroup: the number of stimuli that each group will have. Take in account that the refernce and anchor also have to be counted.
 * Channels: number of channels of your audios.
 * Path: the path where the group folders of the audios are saved
 * FileNames: contains the names of the audios. One field is needed for each group, and they have to be called g1, g2, g3... Inside each group the naming is as follows:
   * ref: name of the reference file.
   * anchor: name of the andhor file.
   * s1: name of the first stimuli.
   * s2: name of the second stimuli.
   * s3: name of the third stimuli.
   * s4: name of the fourth stimuli.
   * s5: name of the fifth stimuli.
   * s6: name of the sixth stimuli.
