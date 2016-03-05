// plugin.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "IPlugin.h"

#include <math.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>//to use cout
//#include <algorithm>
//#include <exception>


using namespace std;


////////////////////////////////////////////////////////////////////////
// A concrete plugin implementation
////////////////////////////////////////////////////////////////////////

// Photo-Reactor Plugin class

//****************************************************************************
//This code has been generated by the Mediachance photo reactor Code generator.


#define AddParameter(N,S,V,M1,M2,T,D) {strcpy (pParameters[N].m_sLabel,S);pParameters[N].m_dValue = V;pParameters[N].m_dMin = M1;pParameters[N].m_dMax = M2;pParameters[N].m_nType = T;pParameters[N].m_dSpecialValue = D;}

#define GetValue(N) (pParameters[N].m_dValue)
#define GetValueY(N) (pParameters[N].m_dSpecialValue)

#define SetValue(N,V) {pParameters[N].m_dValue = V;}

#define GetBOOLValue(N) ((BOOL)(pParameters[N].m_dValue==pParameters[N].m_dMax))

// if it is not defined, then here it is
//#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#define PARAM_SPACIAL 0
#define PARAM_RANGE 1
#define PARAM_ITERATIONS 2

#define NUMBER_OF_USER_PARAMS 3

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


class Plugin1 : public IPlugin	
{
public:

		//Plugin Icon:
	//you can add your own icon by creating 160x100 png file, naming it the same as plugin dll and then placing it in the plugins folder
	//otherwise a generic icon will be use


	//this is the title of the box in workspace. it should be short
	const char* GetTitle () const
	{
		return "Domain Tranform";
	}
	
	// this will appear in the help pane, you can put your credits and short info
	const char* GetDescription () const
	{
		return "Domain Transform for Edge Aware Processing.  The Domain Transform based on a transform that defines an isometry between curves on the 2D image manifold in 5D and the real line.  This filter is based on the concept by Eduardo S. L. Gastal and Manuel M. Oliveira and is a very fast edge preserver filter.";
	}

	// BASIC PARAMETERS
	// number of inputs 0,1 or 2
	int GetInputNumber ()
	{
		return 1;
	}

	// number of outputs 0 or 1
	int GetOutputNumber ()
	{
		return 1;
	}

	int GetBoxColor ()
	{
		return RGB(44,78,119);
	}

	int GetTextColor ()
	{
		return RGB(165,236,255);
	}

	// width of the box in the workspace
	// valid are between 50 and 100
	int GetBoxWidth ()
	{
		return 100;
	}

	// set the flags
	// see the interface builder
	// ex: nFlag = FLAG_FAST_PROCESS | FLAG_HELPER;

	//FLAG_NONE same as zero	Default, no other flags set
	//FLAG_UPDATE_IMMEDIATELY	It is very fast process that can update immediately. When user turns the sliders on UI the left display will update
	//							Use Update Immediately only for fast and single loop processes, for example Desaturate, Levels.
	//FLAG_HELPER				It is an helper object. Helper objects will remain visible in Devices and they can react to mouse messages. Example: Knob, Monitor, Bridge Pin
	//FLAG_BINDING				Binding object, attach to other objects and can change its binding value. It never goes to Process_Data functions.  Example: Knob, Switch, Slider
	//FLAG_DUMMY				It is only for interface but never process any data. Never goes to Process_Data functions. Example: Text note
	//FLAG_SKIPFINAL			Process data only during designing, doesn't process during final export. Example: Monitor, Vectorscope 
	//FLAG_LONGPROCESS			Process that takes > 1s to finish. Long Process will display the Progress dialog and will prevent user from changing values during the process.
	//FLAG_NEEDSIZEDATA		    Process need to know size of original image, the zoom and what part of image is visible in the preview. When set the plugin will receive SetSizeData
	//FLAG_NEEDMOUSE			Process will receive Mouse respond data from the workplace. This is only if your object is interactive, for example Knob, Slider

	int GetFlags ()
	{
		// it is fast process
		//int nFlag = FLAG_UPDATE_IMMEDIATELY;
		
		int nFlag = FLAG_LONGPROCESS;

		return nFlag;
	}


	// User Interface Build
	// there is maximum 29 Parameters

	int GetUIParameters (UIParameters* pParameters)
	{

		// label, value, min, max, type_of_control, special_value
		// use the UI builder in the software to generate this
		
		AddParameter( PARAM_SPACIAL ,"Spacial", 2.0, 1.0, 25.0, TYPE_SLIDER, 0.0);//default  min   max
		AddParameter( PARAM_RANGE ,"Range", 0.2, 0.1, 1.0, TYPE_SLIDER, 0.0);//default  min   max
		AddParameter( PARAM_ITERATIONS ,"Iterations", 1.0, 1.0, 10.0, TYPE_SLIDER, 0.0);
		

		return NUMBER_OF_USER_PARAMS;
	}
	


	// Actual processing function for 1 iterativeArray
	//***************************************************************************************************
	// Both buffers are the same size
	// don't change the IN buffer or things will go bad for other objects in random fashion
	// the pBGRA_out comes already with pre-copied data from pBGRA_in
	// Note: Don't assume the nWidth and nHeight will be every run the same or that it contains the whole image!!!! 
	// This function receives buffer of the actual preview (it can be just a crop of image when zoomed in) and during the final calculation of the full buffer
	// this is where the image processing happens
	virtual void Process_Data (BYTE* pBGRA_out,BYTE* pBGRA_in, int nWidth, int nHeight, UIParameters* pParameters)
	{
		//List of Parameters
		int iterations = GetValue(PARAM_ITERATIONS);
		double sigma_s = GetValue(PARAM_SPACIAL);
		double sigma_r = GetValue(PARAM_RANGE);
		
		int emptyvariable;

		
		int imagesize = nWidth * nHeight * 4;
		float colorspace = 255.0;

		unsigned int heightminus = nHeight - 1;
		unsigned int widthminus = nWidth - 1;

		float* iterativeArray=new float[nWidth * nHeight * 4];
		memset( iterativeArray, 0, sizeof(imagesize) );

		//place our image in a separate array in the 0-1 range, where 0 is darkest and 1 is lightest
		for (unsigned int x = 0; x < nWidth; x++)
		{
			for (unsigned int y = 0; y < nHeight; y++)
			{
				unsigned int nIdx = x * 4 + y * 4 * nWidth;

				//Image0[y][x] = Image [y][x] = my_image -> GetPixelIndex (x, height - y - 1);

				float red = (float)pBGRA_in[nIdx + CHANNEL_R] / colorspace;
				float green = (float)pBGRA_in[nIdx + CHANNEL_G] / colorspace;
				float blue = (float)pBGRA_in[nIdx + CHANNEL_B] / colorspace;

				iterativeArray[nIdx + CHANNEL_R] = red;
				iterativeArray[nIdx + CHANNEL_G] = green;
				iterativeArray[nIdx + CHANNEL_B] = blue;
			}//end x
		}//end y

		// compute derivatives of transformed domain "dct"
		// and a = exp(-sqrt(2) / sigma_H) to the power of "dct"

		//cv::Mat dctx = cv::Mat(height, width-1, CV_64FC1);
		float* dctx=new float[nWidth * nHeight * 4];
		memset( dctx, 0, sizeof(imagesize) );

		double ratio = sigma_s / sigma_r;
		
		for (unsigned int y = 0; y < nHeight; y++)
		{
			for (unsigned int x = 0; x < widthminus; x++)
			{
				unsigned int nIdx1 = x * 4 + y * 4 * nWidth;
				unsigned int nIdx2 = (x + 1) * 4 + y * 4 * nWidth;

				double accumR = 0.0;
				double accumG = 0.0;
				double accumB = 0.0;
				{
					accumR += abs(iterativeArray[nIdx2 + CHANNEL_R] - iterativeArray[nIdx1 + CHANNEL_R]);
					accumG += abs(iterativeArray[nIdx2 + CHANNEL_G] - iterativeArray[nIdx1 + CHANNEL_G]);
					accumB += abs(iterativeArray[nIdx2 + CHANNEL_B] - iterativeArray[nIdx1 + CHANNEL_B]);
				}
				
				dctx[nIdx1 + CHANNEL_R] = 1.0 + ratio * accumR;
				dctx[nIdx1 + CHANNEL_G] = 1.0 + ratio * accumG;
				dctx[nIdx1 + CHANNEL_B] = 1.0 + ratio * accumB;
				//dctx[nIdx1 + CHANNEL_R] = 1.0 + abs(ratio * accumR);
				//dctx[nIdx1 + CHANNEL_G] = 1.0 + abs(ratio * accumG);
				//dctx[nIdx1 + CHANNEL_B] = 1.0 + abs(ratio * accumB);
			}
		}

		float* dcty=new float[nWidth * nHeight * 4];
		memset( dcty, 0, sizeof(imagesize) );

		for (unsigned int x = 0; x < nWidth; x++)
		{
			for (unsigned int y = 0; y < heightminus; y++)
			{
				unsigned int nIdx1 = x * 4 + y * 4 * nWidth;
				unsigned int nIdx2 = x * 4 + (y + 1) * 4 * nWidth;

				double accumR = 0.0;
				double accumG = 0.0;
				double accumB = 0.0;
				{
					accumR += abs(iterativeArray[nIdx2 + CHANNEL_R] - iterativeArray[nIdx1 + CHANNEL_R]);
					accumG += abs(iterativeArray[nIdx2 + CHANNEL_G] - iterativeArray[nIdx1 + CHANNEL_G]);
					accumB += abs(iterativeArray[nIdx2 + CHANNEL_B] - iterativeArray[nIdx1 + CHANNEL_B]);
				}
				
				dcty[nIdx1 + CHANNEL_R] = 1.0 + ratio * accumR;
				dcty[nIdx1 + CHANNEL_G] = 1.0 + ratio * accumG;
				dcty[nIdx1 + CHANNEL_B] = 1.0 + ratio * accumB;
				//dcty[nIdx1 + CHANNEL_R] = 1.0 + abs(ratio * accumR);
				//dcty[nIdx1 + CHANNEL_G] = 1.0 + abs(ratio * accumG);
				//dcty[nIdx1 + CHANNEL_B] = 1.0 + abs(ratio * accumB);
			}
		}

		//tester
		for (int x = 0; x < nWidth; x++)
		{
			for (int y = 0; y < nHeight; y++)
			{
				unsigned int nIdx = x * 4 + y * 4 * nWidth;

				float nR = dctx [nIdx + CHANNEL_R] * colorspace;
				float nG = dctx [nIdx + CHANNEL_G] * colorspace;
				float nB = dctx [nIdx + CHANNEL_B] * colorspace;

			//	pBGRA_out[nIdx + CHANNEL_R] = CLAMP255(nR);
			//	pBGRA_out[nIdx + CHANNEL_G] = CLAMP255(nG);
			//	pBGRA_out[nIdx + CHANNEL_B] = CLAMP255(nB);
			}
		}


		/*
		 // Apply recursive folter maxiter times
		img.convertTo(out, CV_MAKETYPE(CV_64F, dim));
		for(int i=0; i<maxiter; i++) 
		{
			double sigma_H = sigma_s * sqrt(3.0) * pow(2.0, maxiter - i - 1) / sqrt(pow(4.0, maxiter) - 1.0);
			recursiveFilterHorizontal(out, dctx, sigma_H);
			recursiveFilterVertical(out, dcty, sigma_H);
		}
		*/




		//This is our Domain Transform routine
		for(unsigned int i=0; i<iterations; i++) //for i = 0:num_iterations - 1
		{
			double sigma_H = sigma_s * sqrt(3.0) * pow(2.0, iterations - i - 1) / sqrt(pow(4.0, iterations) - 1.0);

			double a = exp(-sqrt(2.0) / sigma_H);

			float* horizontal=new float[nWidth * nHeight * 4];
			memset( horizontal, 0, sizeof(imagesize) );

			float* vertical=new float[nWidth * nHeight * 4];
			memset( vertical, 0, sizeof(imagesize) );

			for (unsigned int x = 0; x < widthminus; x++)
			{
				for (unsigned int y = 0; y < nHeight; y++)
				{
					unsigned int nIdx1 = x * 4 + y * 4 * nWidth;
					//unsigned int nIdx2 = (x + 1) * 4 + y * 4 * nWidth;

					//V.at<double>(y, x) = pow(a, V.at<double>(y, x));
					horizontal[nIdx1 + CHANNEL_R] = powf(a, dctx[nIdx1 + CHANNEL_R]);
					horizontal[nIdx1 + CHANNEL_G] = powf(a, dctx[nIdx1 + CHANNEL_G]);
					horizontal[nIdx1 + CHANNEL_B] = powf(a, dctx[nIdx1 + CHANNEL_B]);
				}
			}

			for (unsigned int x = 0; x < nWidth; x++)
			{
				for (unsigned int y = 0; y < heightminus; y++)
				{
					unsigned int nIdx1 = x * 4 + y * 4 * nWidth;

					vertical[nIdx1 + CHANNEL_R] = powf(a, dcty[nIdx1 + CHANNEL_R]);
					vertical[nIdx1 + CHANNEL_G] = powf(a, dcty[nIdx1 + CHANNEL_G]);
					vertical[nIdx1 + CHANNEL_B] = powf(a, dcty[nIdx1 + CHANNEL_B]);
				}
			}


			{//recursiveFilterHorizontal

				//forwards Horizontal
				for (unsigned int y = 0; y < nHeight; y++)//for(int y=0; y<height; y++) {
				{
					for (unsigned int x = 1; x < nWidth; x++)//for(int x=1; x<width; x++) {
					{
						unsigned int nIdx1 = x * 4 + y * 4 * nWidth;
						unsigned int nIdx2 = (x - 1) * 4 + y * 4 * nWidth;

						//double p = V.at<double>(y, x-1);
						double pR = horizontal[nIdx2 + CHANNEL_R];
						double pG = horizontal[nIdx2 + CHANNEL_G];
						double pB = horizontal[nIdx2 + CHANNEL_B];

						//double val1 = out.at<double>(y, x*dim+c);
						double val1R = iterativeArray[nIdx1 + CHANNEL_R];
						double val1G = iterativeArray[nIdx1 + CHANNEL_G];
						double val1B = iterativeArray[nIdx1 + CHANNEL_B];

						//double val2 = out.at<double>(y, (x-1)*dim+c);
						double val2R = iterativeArray[nIdx2 + CHANNEL_R];
						double val2G = iterativeArray[nIdx2 + CHANNEL_G];
						double val2B = iterativeArray[nIdx2 + CHANNEL_B];

						//out.at<double>(y, x*dim+c) = val1 + p * (val2 - val1);
						iterativeArray[nIdx1 + CHANNEL_R] = val1R + pR * (val2R - val1R);
						iterativeArray[nIdx1 + CHANNEL_G] = val1G + pG * (val2G - val1G);
						iterativeArray[nIdx1 + CHANNEL_B] = val1B + pB * (val2B - val1B);
					}//end x
				
					//backwards horizontal
					for (unsigned int x = nWidth - 2; x > 0; x--)
					{
						unsigned int nIdx1 = x * 4 + y * 4 * nWidth;
						unsigned int nIdx2 = (x + 1) * 4 + y * 4 * nWidth;

						double pR = horizontal[nIdx1 + CHANNEL_R];
						double pG = horizontal[nIdx1 + CHANNEL_G];
						double pB = horizontal[nIdx1 + CHANNEL_B];

						//double val1 = out.at<double>(y, x*dim+c);
						double val1R = iterativeArray[nIdx1 + CHANNEL_R];
						double val1G = iterativeArray[nIdx1 + CHANNEL_G];
						double val1B = iterativeArray[nIdx1 + CHANNEL_B];

						//double val2 = out.at<double>(y, (x+1)*dim+c);
						double val2R = iterativeArray[nIdx2 + CHANNEL_R];
						double val2G = iterativeArray[nIdx2 + CHANNEL_G];
						double val2B = iterativeArray[nIdx2 + CHANNEL_B];

						//out.at<double>(y, x*dim+c) = val1 + p * (val2 - val1);
						iterativeArray[nIdx1 + CHANNEL_R] = val1R + pR * (val2R - val1R);
						iterativeArray[nIdx1 + CHANNEL_G] = val1G + pG * (val2G - val1G);
						iterativeArray[nIdx1 + CHANNEL_B] = val1B + pB * (val2B - val1B);
					}//end x
				}//end y
			}// end recursiveFilterHorizontal



			{//recursiveFilterVertical
				//forwards vertical
				for (unsigned int x = 0; x < nWidth; x++)
				{
					for (unsigned int y = 1; y < nHeight; y++)
					{						
						unsigned int nIdx1 = x * 4 + y * 4 * nWidth;
						unsigned int nIdx2 = x * 4 + (y - 1) * 4 * nWidth;

						double pR = vertical[nIdx2 + CHANNEL_R];
						double pG = vertical[nIdx2 + CHANNEL_G];
						double pB = vertical[nIdx2 + CHANNEL_B];

						//double val1 = out.at<double>(y, x*dim+c);
						double val1R = iterativeArray[nIdx1 + CHANNEL_R];
						double val1G = iterativeArray[nIdx1 + CHANNEL_G];
						double val1B = iterativeArray[nIdx1 + CHANNEL_B];

						//double val2 = out.at<double>(y+1, x*dim+c);
						double val2R = iterativeArray[nIdx2 + CHANNEL_R];
						double val2G = iterativeArray[nIdx2 + CHANNEL_G];
						double val2B = iterativeArray[nIdx2 + CHANNEL_B];

						//out.at<double>(y, x*dim+c) = val1 + p * (val2 - val1);
						iterativeArray[nIdx1 + CHANNEL_R] = val1R + pR * (val2R - val1R);
						iterativeArray[nIdx1 + CHANNEL_G] = val1G + pG * (val2G - val1G);
						iterativeArray[nIdx1 + CHANNEL_B] = val1B + pB * (val2B - val1B);
					}//end y

					//backwards vertical
					for (unsigned int y = nHeight - 2; y > 0; y--)
					{
						unsigned int nIdx1 = x * 4 + y * 4 * nWidth;
						unsigned int nIdx2 = x * 4 + (y + 1) * 4 * nWidth;
						//working this section

						double pR = vertical[nIdx1 + CHANNEL_R];
						double pG = vertical[nIdx1 + CHANNEL_G];
						double pB = vertical[nIdx1 + CHANNEL_B];

						//double val1 = out.at<double>(y, x*dim+c);
						double val1R = iterativeArray[nIdx1 + CHANNEL_R];
						double val1G = iterativeArray[nIdx1 + CHANNEL_G];
						double val1B = iterativeArray[nIdx1 + CHANNEL_B];

						//double val2 = out.at<double>(y+1, x*dim+c);
						double val2R = iterativeArray[nIdx2 + CHANNEL_R];
						double val2G = iterativeArray[nIdx2 + CHANNEL_G];
						double val2B = iterativeArray[nIdx2 + CHANNEL_B];

						//out.at<double>(y, x*dim+c) = val1 + p * (val2 - val1);
						iterativeArray[nIdx1 + CHANNEL_R] = val1R + pR * (val2R - val1R);
						iterativeArray[nIdx1 + CHANNEL_G] = val1G + pG * (val2G - val1G);
						iterativeArray[nIdx1 + CHANNEL_B] = val1B + pB * (val2B - val1B);
					}//end y
				}//end x
			}



			delete [] horizontal;
			delete [] vertical;

		}//end iterations

		
#pragma region

		delete [] dctx;
		delete [] dcty;

		

		for (int x = 0; x < nWidth; x++)
		{
			for (int y = 0; y < nHeight; y++)
			{
					unsigned int nIdx = x * 4 + y * 4 * nWidth;

					float nR = iterativeArray [nIdx + CHANNEL_R] * colorspace;
					float nG = iterativeArray [nIdx + CHANNEL_G] * colorspace;
					float nB = iterativeArray [nIdx + CHANNEL_B] * colorspace;

					pBGRA_out[nIdx + CHANNEL_R] = CLAMP255(nR);
					pBGRA_out[nIdx + CHANNEL_G] = CLAMP255(nG);
					pBGRA_out[nIdx + CHANNEL_B] = CLAMP255(nB);
			}
		}

		delete [] iterativeArray;
#pragma endregion
		


	}//end routine




	// actual processing function for 2 inputs
	//********************************************************************************
	// all buffers are the same size
	// don't change the IN buffers or things will go bad
	// the pBGRA_out comes already with copied data from pBGRA_in1
	virtual void Process_Data2 (BYTE* pBGRA_out, BYTE* pBGRA_in1, BYTE* pBGRA_in2, int nWidth, int nHeight, UIParameters* pParameters)
	{

	}


	//*****************Drawing functions for the BOX *********************************
	//how is the drawing handled
	//DRAW_AUTOMATICALLY	the main program will fully take care of this and draw a box, title, socket and thumbnail
	//DRAW_SIMPLE_A			will draw a box, title and sockets and call CustomDraw
	//DRAW_SIMPLE_B			will draw a box and sockets and call CustomDraw
	//DRAW_SOCKETSONLY      will call CustomDraw and then draw sockets on top of it
	
	// highlighting rectangle around is always drawn except for DRAW_SOCKETSONLY

	virtual int GetDrawingType ()
	{

		int nType = DRAW_AUTOMATICALLY;

		return nType;

	}


	// Custom Drawing
	// custom drawing function called when drawing type is different than DRAW_AUTOMATICALLY
	// it is not always in real pixels but scaled depending on where it is drawn
	// the scale could be from 1.0 to > 1.0
	// so you always multiply the position, sizes, font size, line width with the scale
	
	virtual void CustomDraw (HDC hDC, int nX,int nY, int nWidth, int nHeight, float scale, BOOL bIsHighlighted, UIParameters* pParameters)
	{
	}


	//************ Optional Functions *****************************************************************************************
	// those functions are not necessary for normal effect, they are mostly for special effects and objects


	// Called when FLAG_HELPER set. 
	// When UI data changed (user turned knob) this function will be called as soon as user finish channging the data
	// You will get the latest parameters and also which parameter changed
	// Normally for effects you don't have to do anything here because you will get the same parameters in the process function
	// It is only for helper objects that may not go to Process Data 
	BOOL UIParametersChanged (UIParameters* pParameters, int nParameter)
	{
		
		return FALSE;
	}

	// when button is pressed on UI, this function will be called with the parameter and sub button (for multi button line)
	BOOL UIButtonPushed (int nParam, int nSubButton, UIParameters* pParameters)
	{

		return TRUE;
	}


	// Called when FLAG_NEEDSIZEDATA set
	// Called before each calculation (Process_Data)
	// If your process depends on a position on a frame you may need the data to correctly display it because Process_Data receives only a preview crop
	// Most normal effects don't depend on the position in frame so you don't need the data
	// Example: drawing a circle at a certain position requires to know what is displayed in preview or the circle will be at the same size and position regardless of zoom
	
	// Note: Even if you need position but you don't want to mess with the crop data, just ignore it and pretend the Process_Data are always of full image (they are not). 
	// In worst case this affects only preview when using zoom. The full process image always sends the whole data

	// nOriginalW, nOriginalH - the size of the original - full image. If user sets Resize on iterativeArray - this will be the resized image
	// nPreviewW, nPreviewH   - this is the currently processed preview width/height - it is the same that Process_Data will receive
	//                        - in full process the nPreviewW, nPreviewH is equal nOriginalW, nOriginalH
	// Crop X1,Y1,X2,Y2       - relative coordinates of preview crop rectangle in <0...1>, for full process they are 0,0,1,1 (full rectangle)	
	// dZoom                  - Zoom of the Preview, for full process the dZoom = 1.0
	void SetSizeData(int nOriginalW, int nOriginalH, int nPreviewW, int nPreviewH, double dCropX1, double dCropY1, double dCropX2, double dCropY2, double dZoom)
	{

		// so if you need the position and zoom, this is the place to get it.
		// Note: because of IBM wisdom the internal bitmaps are on PC always upside down, but the coordinates are not


	}


	// ***** Mouse handling on workplace *************************** 
	// only if FLAG_NEEDMOUSE is set
	//****************************************************************
	//this is for special objects that need to receive mouse, like a knob or slider on workplace
	// normally you use this for FLAG_BINDING objects

	// in coordinates relative to top, left corner of the object (0,0)
	virtual BOOL MouseButtonDown (int nX, int nY, int nWidth, int nHeight, UIParameters* pParameters)
	{
		
		// return FALSE if not handled
		// return TRUE if handled
		return FALSE;
	}

	// in coordinates relative to top, left corner of the object (0,0)
	virtual BOOL MouseMove (int nX, int nY, int nWidth, int nHeight, UIParameters* pParameters)
	{
	

		return FALSE;
	}
	
	// in coordinates relative to top, left corner of the object (0,0)
	virtual BOOL MouseButtonUp (int nX, int nY, int nWidth, int nHeight, UIParameters* pParameters)
	{
		
		// Note: if we changed data and need to recalculate the flow we need to return TRUE

		// return FALSE if not handled
		// return TRUE if handled
		
		return TRUE;
	}


};

extern "C"
{
	// Plugin factory function
	__declspec(dllexport) IPlugin* Create_Plugin ()
	{
		//allocate a new object and return it
		return new Plugin1 ();
	}
	
	// Plugin cleanup function
	__declspec(dllexport) void Release_Plugin (IPlugin* p_plugin)
	{
		//we allocated in the factory with new, delete the passed object
		delete p_plugin;
	}
	
}


// this is the name that will appear in the object library
extern "C" __declspec(dllexport) char* GetPluginName()
{
	return "!Andy's Domain Tranform";	
}


// This MUST be unique string for each plugin so we can save the data

extern "C" __declspec(dllexport) char* GetPluginID()
{
	
	
	return "com.lumafilters.DomainTranform";
	
}


// category of plugin, for now the EFFECT go to top library box, everything else goes to the middle library box
extern "C" __declspec(dllexport) int GetCategory()
{
		
	return CATEGORY_EFFECT;
	
}
