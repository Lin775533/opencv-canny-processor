/**
 * Camera Canny Edge Detection
 * Modified for multiple frame processing and performance profiling
 */
#include <ctime>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <numeric>
#include <vector>
#include <opencv2/opencv.hpp>
#include "canny_util.h"
#include <iomanip>

using namespace std;
using namespace cv;
using namespace std::chrono;

/* Camera resolution settings */
#define WIDTH 640
#define HEIGHT 480
#define MAX_FRAMES 150  // Number of frames to capture (~5 seconds at 30fps)


int main(int argc, char **argv)
{
    char* dirfilename;        /* Name of the output gradient direction image */
    char outfilename[128];    /* Name of the output "edge" image */
    char composedfname[128];  /* Name of the output "direction" image */
    unsigned char *edge;      /* The output edge image */
    float sigma,              /* Standard deviation of the gaussian kernel. */
          tlow,               /* Fraction of the high threshold in hysteresis. */
          thigh;              /* High hysteresis threshold control. */

    /****************************************************************************
     * Get the command line arguments.
     ****************************************************************************/
    if(argc < 4){
        fprintf(stderr,"\n<USAGE> %s sigma tlow thigh [writedirim]\n",argv[0]);
        fprintf(stderr,"      sigma:      Standard deviation of the gaussian blur kernel.\n");
        fprintf(stderr,"      tlow:       Fraction (0.0-1.0) of the high edge strength threshold.\n");
        fprintf(stderr,"      thigh:      Fraction (0.0-1.0) of the distribution of non-zero edge strengths.\n");
        fprintf(stderr,"      writedirim: Optional argument to output a floating point direction image.\n\n");
        exit(1);
    }

    sigma = atof(argv[1]);
    tlow = atof(argv[2]);
    thigh = atof(argv[3]);

    if(argc == 5) dirfilename = (char *) "dummy";
    else dirfilename = NULL;

    // Define the GStreamer pipeline for libcamera
    string pipeline = 
        "libcamerasrc ! "
        "video/x-raw,format=NV12,width=640,height=480,framerate=30/1 ! "
        "videoconvert ! "
        "appsink drop=true";  // appsink is required for feeding frames into OpenCV

    VideoCapture cap(pipeline, CAP_GSTREAMER);
    if(!cap.isOpened()) {
        cerr << "Error: Could not open libcamera pipeline via GStreamer." << endl;
        return -1;
    }

    Mat frame, grayframe;
    vector<double> frame_times;  // Store processing times for each frame
    int frame_count = 0;

    // Preview window before processing
    cout << "[INFO] Press ESC to start Canny edge detection..." << endl;
    for(;;) {
        cap >> frame;
        if(frame.empty()) break;
        imshow("[RAW] Preview", frame);
        if(waitKey(10) == 27) break;
    }

    // Process multiple frames
    auto total_start = high_resolution_clock::now();
    
    while(frame_count < MAX_FRAMES) {
        auto frame_start = high_resolution_clock::now();
        
        // Capture frame
        cap >> frame;
        if(frame.empty()) {
            cerr << "Error: Failed to capture frame" << endl;
            break;
        }
        
        // Convert to grayscale
        cvtColor(frame, grayframe, COLOR_BGR2GRAY);  // Using OpenCV 4.x constant
        unsigned char* image = grayframe.data;

        // Process frame with Canny
        if(VERBOSE) printf("Processing frame %d/%d\n", frame_count + 1, MAX_FRAMES);
        
        canny(image, HEIGHT, WIDTH, sigma, tlow, thigh, &edge, dirfilename);

        // Save processed frame
        sprintf(outfilename, "frame%03d.pgm", frame_count + 1);
        if(write_pgm_image(outfilename, edge, HEIGHT, WIDTH, NULL, 255) == 0) {
            fprintf(stderr, "Error writing frame %d\n", frame_count + 1);
            continue;
        }

        // Display processed frame
        Mat edge_frame(HEIGHT, WIDTH, CV_8UC1, edge);
        imshow("[EDGE] Processed Frame", edge_frame);
        waitKey(1);  // Brief display

        // Calculate frame processing time
        auto frame_end = high_resolution_clock::now();
        duration<double> frame_duration = frame_end - frame_start;
        frame_times.push_back(frame_duration.count());

        // Display progress
        cout << "\rProcessed frame " << frame_count + 1 << "/" << MAX_FRAMES << flush;
        
        frame_count++;
        
        // Free edge memory for next iteration
        free(edge);
    }
    
    auto total_end = high_resolution_clock::now();

    // Calculate and display timing statistics
    duration<double> total_duration = total_end - total_start;
    double avg_fps = frame_count / total_duration.count();
    double avg_frame_time = accumulate(frame_times.begin(), frame_times.end(), 0.0) / frame_count;

    // CPU time calculation
    clock_t cpu_time = clock();
    double cpu_time_seconds = static_cast<double>(cpu_time) / CLOCKS_PER_SEC;

    cout << "\n\nPerformance Statistics:" << endl;
    cout << "========================" << endl;
    cout << "Total frames processed: " << frame_count << endl;
    cout << "Wall time (total): " << fixed << setprecision(3) << total_duration.count() << " seconds" << endl;
    cout << "CPU time: " << fixed << setprecision(3) << cpu_time_seconds << " seconds" << endl;
    cout << "Average time per frame: " << fixed << setprecision(3) << avg_frame_time << " seconds" << endl;
    cout << "Average FPS: " << fixed << setprecision(2) << avg_fps << endl;

    // Instructions for video encoding
    cout << "\nTo encode processed frames into video, run:" << endl;
    cout << "ffmpeg -i frame%03d.pgm -pix_fmt yuvj420p frame_vid.h264" << endl;

    cout << "\n[INFO] Press ESC to exit..." << endl;
    while(waitKey(10) != 27);

    // Cleanup
    cap.release();
    destroyAllWindows();

    return 0;
}
