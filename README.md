# Tone Player
Tone Player reads a melody defined in a text file and generates audio
from it. The program plays the melody, visualises the tones and saves
the audio into a WAV file.

Example usage (the file `song.txt` contains a sample melody for the program):
```
./player song.txt
```

## Audio format
First two lines of the file define the base frequency (in Hertz)
and audio duration (milliseconds).

Example:
```
110
18000
```

The remaining lines describe the tones of the melody. Each line
has this format:
```
<TONE> <DURATION> <VOLUME> <TIME>
```

- `TONE` is a number which determines how many half-tones above the base frequency the tone is
    - It can also be "rest" which represents silence
- `DURATION` is a duration of the tone in milliseconds
- `VOLUME` is a volume of the tone (0 to 1000)
- `TIME` is the time when the tone should start playing (in milliseconds)
    - It can also be "after" which means the tone starts playing right after the tone defined on the previous line
