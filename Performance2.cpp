// Visual Studio 2017 version.

#include "stdafx.h"
#include "Performance2.h"

#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Timer - used to established precise timings for code.
class TIMER
{
	LARGE_INTEGER t_;

	__int64 current_time_;

	public:
		TIMER()	// Default constructor. Initialises this timer with the current value of the hi-res CPU timer.
		{
			QueryPerformanceCounter(&t_);
			current_time_ = t_.QuadPart;
		}

		TIMER(const TIMER &ct)	// Copy constructor.
		{
			current_time_ = ct.current_time_;
		}

		TIMER& operator=(const TIMER &ct)	// Copy assignment.
		{
			current_time_ = ct.current_time_;
			return *this;
		}

		TIMER& operator=(const __int64 &n)	// Overloaded copy assignment.
		{
			current_time_ = n;
			return *this;
		}

		~TIMER() {}		// Destructor.

		static __int64 get_frequency()
		{
			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency); 
			return frequency.QuadPart;
		}

		__int64 get_time() const
		{
			return current_time_;
		}

		void get_current_time()
		{
			QueryPerformanceCounter(&t_);
			current_time_ = t_.QuadPart;
		}

		inline bool operator==(const TIMER &ct) const
		{
			return current_time_ == ct.current_time_;
		}

		inline bool operator!=(const TIMER &ct) const
		{
			return current_time_ != ct.current_time_;
		}

		__int64 operator-(const TIMER &ct) const		// Subtract a TIMER from this one - return the result.
		{
			return current_time_ - ct.current_time_;
		}

		inline bool operator>(const TIMER &ct) const
		{
			return current_time_ > ct.current_time_;
		}

		inline bool operator<(const TIMER &ct) const
		{
			return current_time_ < ct.current_time_;
		}

		inline bool operator<=(const TIMER &ct) const
		{
			return current_time_ <= ct.current_time_;
		}

		inline bool operator>=(const TIMER &ct) const
		{
			return current_time_ >= ct.current_time_;
		}
};

CWinApp theApp;  // The one and only application object

using namespace std;
using namespace cv;



	//--------------------------------------------------------------------------------------
	// My Code

	int Num_Threads = std::thread::hardware_concurrency(); // Ammunt of possible threads the compouter can run
	std::mutex mtx; // Lock mutex


	// A que of the images
	std::queue<Mat> img_que;

	// Queue of filenames
	std::queue<std::string> filenames;

	// Function to let the image be rotate around a angle
	Mat rotate(Mat src, double angle)
	{
		Mat dst;
		Point2f pt(src.cols / 2., src.rows / 2.);
		Mat r = getRotationMatrix2D(pt, angle, 1.0);
		warpAffine(src, dst, r, Size(src.cols, src.rows));
		return dst;
	}

	// We will edit the images here
	void Edit_Image(Mat img) {

		// rotates the images
		img = rotate(img, 90);

		// Dubble the size of a image with bilinear interpolation which is the last variable
		cv::resize(img, img, cv::Size(img.cols * 2, img.rows * 2), 0, 0, 1);

		// Brighten the image by 20%
		img.convertTo(img, -1, 1, 20);

		std::string filename = filenames.front();
		filenames.pop();
		// Save the Image
		imwrite("SavedImages/IMG_" + filename + ".png", img);
	}

	// look for a job infinatley
	void look_for_job(std::queue<Mat> img_queue) {

		Mat img;
		bool Working = true;
		while (Working == true) {
			if (!img_que.empty()) {
				Working = true;
				mtx.lock();
				img = img_que.front();
				img_que.pop();
				mtx.unlock();
				Edit_Image(img);	
			}
			else
			{
				Working = false;
			}
		}
	}

	//-------------------------------------------------------------------------------------------------------
	// My Code


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize Microsoft Foundation Classes, and print an error if failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		// Application starts here...

		// Time the application's execution time.
		TIMER start;	// DO NOT CHANGE THIS LINE. Timing will start here.

		//--------------------------------------------------------------------------------------
		// Insert your code from here...

		//Creat the thread pool
		std::vector<std::thread> thread_pool;
		// Read FileNames from the folder
		std::vector<cv::String> fn;
		// Load any image that has a JPG in it
		glob("SampleImages/*JPG", fn, false);

		// Read the images 
		for (int i = 0; i < fn.size(); i++) {
			img_que.push(imread(fn[i], IMREAD_GRAYSCALE));
			filenames.push(std::to_string(i+1));
		};
		// Populate the Thread pool with a Look for job function
		for (int i = 0; i < Num_Threads; i++) {
			thread_pool.push_back(std::thread(look_for_job, img_que));
		}
		// Wait until the threads are finished 
		for (int i = 0; i < Num_Threads; i++) {
			thread_pool.at(i).join();


			//-------------------------------------------------------------------------------------------------------
			// How long did it take?...   DO NOT CHANGE FROM HERE...

			TIMER end;

			TIMER elapsed;

			elapsed = end - start;

			__int64 ticks_per_second = start.get_frequency();

			// Display the resulting time...

			double elapsed_seconds = (double)elapsed.get_time() / (double)ticks_per_second;

			cout << "Elapsed time (seconds): " << elapsed_seconds;
			cout << endl;
			cout << "Press a key to continue" << endl;

			char c;
			cin >> c;
		}
	}
	return nRetCode;
}
