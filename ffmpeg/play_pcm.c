#include "play_pcm.h"

/*------------------------------------------------
 open an PCM device and set following parameters:
 1.  access mode  (SND_PCM_ACCESS_RW_INTERLEAVED)
 2.  PCM format    (SND_PCM_FORMAT_S16_LE)
 3.  number of channels  ------  unsigned int nchan
 4.  sampling rate  ----- unsigned int srate



Return:
a	0  :  OK
	<0 :  fails
--------------------------------------------------*/
int prepare_pcm_device(unsigned int nchan, unsigned int srate)
{
	int rc;
        snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;
        int dir=0;

	//----- open PCM device for playblack
	rc=snd_pcm_open(&g_ffpcm_handle,"default",SND_PCM_STREAM_PLAYBACK,0);
	if(rc<0)
	{
		fprintf(stderr,"unable to open pcm device: %s\n",snd_strerror(rc));
		return rc;
	}

	//----- allocate a hardware parameters object
	snd_pcm_hw_params_alloca(&params);

	//----- fill it in with default values
	snd_pcm_hw_params_any(g_ffpcm_handle, params);

	//<<<<<<<<<<<<<<<<<<<       set hardware parameters     >>>>>>>>>>>>>>>>>>>>>>
	//----- interleaved mode
	snd_pcm_hw_params_set_access(g_ffpcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	//----- noninterleaved mode
//	snd_pcm_hw_params_set_access(g_ffpcm_handle, params, SND_PCM_ACCESS_RW_NONINTERLEAVED);

	//----- signed 16-bit little-endian format
	snd_pcm_hw_params_set_format(g_ffpcm_handle, params, SND_PCM_FORMAT_S16_LE);

	//----- two channels
	snd_pcm_hw_params_set_channels(g_ffpcm_handle, params, nchan);

	//----- sampling rate
	snd_pcm_hw_params_set_rate_near(g_ffpcm_handle, params, &srate, &dir);
	if(dir != 0)
		printf(" Actual sampling rate is set to %d HZ!\n",srate);


	rc=snd_pcm_hw_params(g_ffpcm_handle,params);
	if(rc<0) //rc=0 on success
	{
		fprintf(stderr,"unable to set hw parameter: %s\n",snd_strerror(rc));
		return rc;
	}

	//----- get period size
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);
	printf("snd pcm period size = %d frames\n",(int)frames);


	return rc;
}

/*----------------------------------------------------
  close pcm device and free resources

----------------------------------------------------*/
void close_pcm_device(void)
{
	if(g_ffpcm_handle != NULL) {
		snd_pcm_drain(g_ffpcm_handle);
		snd_pcm_close(g_ffpcm_handle);
	}
}



/*---------------------------------------------------------
send buffer data to pcm play device 
PCM access mode MUST have been set properly in open_pcm_device().

buffer ---  point to pcm data buffer
nf     ---  number of frames

Return:
	>0    OK
	<0   fails
----------------------------------------------------------*/
void  play_pcm_buff(uint8_t * buffer, int nf)
{
	int rc;

        rc=snd_pcm_writei(g_ffpcm_handle,buffer,(snd_pcm_uframes_t)nf ); //write to hw to playback 
        if (rc == -EPIPE)
        {
            //EPIPE means underrun
            fprintf(stderr,"underrun occurred\n");
            snd_pcm_prepare(g_ffpcm_handle);
        }
	else if(rc<0)
        {
        	fprintf(stderr,"error from writei():%s\n",snd_strerror(rc));
        }
        else if (rc != nf)
        {
                fprintf(stderr,"short write, write %d of total %d frames\n",rc,nf);
        }

}