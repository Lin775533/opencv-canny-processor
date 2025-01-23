# Canny Edge Detection Camera System

A real-time camera-based Canny edge detection system implemented using OpenCV and C++.

## Features
- Real-time video capture using libcamera
- Canny edge detection processing
- Performance metrics tracking (CPU time vs Wall time)
- Frame-by-frame processing with up to 150 frames
- Video encoding capability

## Prerequisites
- OpenCV 4.x
- GStreamer
- libcamera
- C++ compiler
- ffmpeg (for video encoding)

## Installation
1. Clone the repository:
```bash
git clone https://github.com/YourUsername/canny-edge-camera-YCLin.git
cd canny-edge-camera-YCLin
```

2. Compile the program:
```bash
g++ canny_util.c camera_canny.cpp -o camera_canny -I. `pkg-config --cflags --libs opencv4`
```

## Usage
1. Run the program:
```bash
./camera_canny <sigma> <tlow> <thigh>
```
Example:
```bash
./camera_canny 1.0 0.5 0.75
```

2. Press ESC to start processing
3. The program will automatically process 150 frames
4. Results will be saved as individual PGM files

### Creating Video from Frames
To encode processed frames into video:
```bash
ffmpeg -i frame%03d.pgm -pix_fmt yuvj420p frame_vid.h264
```

## Project Structure
- `camera_canny.cpp`: Main program file
- `canny_util.c`: Canny edge detection implementation
- `report.md`: Detailed project report
- `README.md`: Project documentation

## Author
- YU-CHUN, LIN

## License
This project is licensed under the MIT License - see the LICENSE file for details
