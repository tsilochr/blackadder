package eu.pursuit.voiceapp;
/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author dimitrisdiamantis
 */


import java.util.Arrays;
import java.util.concurrent.BlockingQueue;

import java.util.logging.Level;
import java.util.logging.Logger;
import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.TargetDataLine;

public class JavaSoundRecorder implements Runnable {
	
	AudioFormat audioFormat = new AudioFormat(AudioFormat.Encoding.PCM_SIGNED,
			8000, 16, 2, 4, 8000, false);

	DataLine.Info dataLineInfo = new DataLine.Info(TargetDataLine.class,
			audioFormat);

	TargetDataLine target = null;

	private BlockingQueue<byte []> queue;

	public JavaSoundRecorder(BlockingQueue<byte []> queue) {
		this.queue = queue;
	}

	public void run() {

		/*try {
			target = (TargetDataLine) AudioSystem.getLine(dataLineInfo);
			target.open(audioFormat, Configuration.BUFF_SIZE);
			target.start();
                     
                     
		} catch (LineUnavailableException e) {
			e.printStackTrace();
		}*/
		
		byte[] abBuffer = new byte[Configuration.BUFF_SIZE];
                Arrays.fill(abBuffer, (byte)4);
                while(true){
                    try {
                        Thread.sleep(500);
                    } catch (InterruptedException ex) {
                       ex.printStackTrace();
                    }
                    this.queue.offer(abBuffer);
                }
		/*while (true) {

			int nBytesRead = target.read(abBuffer, 0, abBuffer.length);			
			byte[] data = Arrays.copyOf(abBuffer, nBytesRead);						
		b	
			boolean ok = this.queue.offer(data);	
		}*/
	}
}
