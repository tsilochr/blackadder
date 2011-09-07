package eu.pursuit.voiceapp;
import java.util.concurrent.BlockingQueue;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

public class JavaSoundPlayer implements Runnable {
	AudioFormat audioFormat = new AudioFormat(AudioFormat.Encoding.PCM_SIGNED,
			8000, 16, 2, 4, 8000, false);
	byte[] abBuffer = new byte[Configuration.BUFF_SIZE];
	DataLine.Info dataLineInfo = new DataLine.Info(SourceDataLine.class,
			audioFormat);

	private SourceDataLine source = null;	
	private BlockingQueue<byte []> queue;

	public JavaSoundPlayer(BlockingQueue<byte []> queue) {
		this.queue = queue;
	}

	public void run() {
		System.out.println("Connecting...");		

		/*try {
			source = (SourceDataLine) AudioSystem.getLine(dataLineInfo);
			source.open(audioFormat, abBuffer.length);
			source.start();
		} catch (LineUnavailableException e) {
			e.printStackTrace();
		}*/
		System.out.println("done");

		while (true) {
			try {
				byte [] data = queue.take();
				//source.write(data, 0, data.length);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
}
