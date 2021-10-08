# mushraTests

mushraTests is a powerful tool to do MUSHRA tests (ITU-R BS.1534-3) with multichannel sound sources. It has been developed for the *Comparison of the spatial properties of Ambisonics versus the Spatial Wavelet Format (SWF)* master thesis by Rubén Eguinoa Cabrito which has been presented to the International Conference on Immersive and 3D Audio 2021 (I3DA).

![Localization tests interface](https://github.com/iRubec/mushraTests/blob/main/images/interfacePos.PNG)

![Source width tests interface](https://github.com/iRubec/mushraTests/blob/main/images/interfaceWidth.PNG)

You can do tests with a maximun of 6 stimuli plus the reference and the anchor. All the results will be saved in a xml file.

## How to design a test

To design your own test, you need to fill the *testDescription.json* file. This file will contain all the information needed to run correctly the test:
 * Desc: the description of the test.
 * Groups: the number of groups of your test.
 * StimulisPerGroup: the number of stimuli that each group will have. Take in account that the refernce and anchor have to be counted.
 * Channels: number of channels of your audios.
 * Path: the path where the group folders of the audios are saved
 * FileNames: contains the names of the audios. One field is needed for each group, and they have to be called g1, g2, g3... Inside each group the naming is as follows:
   * ref: name of the reference file.
   * anchor: name of the andhor file.
   * s1: name of the first stimulus.
   * s2: name of the second stimulus.
   * s3: name of the third stimulus.
   * s4: name of the fourth stimulus.
   * s5: name of the fifth stimulus.
   * s6: name of the sixth stimulus.
   
   If in your test the number of stimuli is below 6, just do not fill the file field.
   
## Results reading

The results are saved in a xml file as follows: each <TEST> tag represent one group of the test (in the same order as presented in the JSON file). Inside this <TEST> tag, one tag for each stimuli is created named with a letter from A to H, where the stimuli name (ref, anch, s1, s2, s3, s4, s5 or s6) and the given answer is attributed. Note that the order of appearence in each gorup is also the order that the user has have during that group.
  
```
<testData>
  <TEST>
    <A>
      <tech>anch</tech>
      <ans>51.000000</ans>
    </A>
    <B>
      <tech>s4</tech>
      <ans>93.000000</ans>
    </B>
    <C>
      <tech>s1</tech>
      <ans>43.000000</ans>
    </C>
    <D>
      <tech>s2</tech>
      <ans>78.000000</ans>
    </D>
    <E>
      <tech>ref</tech>
      <ans>100.000000</ans>
    </E>
    <F>
      <tech>s5</tech>
      <ans>61.000000</ans>
    </F>
    <G>
      <tech>s3</tech>
      <ans>57.000000</ans>
    </G>
    <H>
      <tech>s6</tech>
      <ans>84.000000</ans>
    </H>
  </TEST>
  <TEST>
    .
    .
    .
    .
    .
</testData>
```
