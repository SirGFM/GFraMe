package com.wordpress.gfmgamecorner.bugsquasher;

import org.libsdl.app.SDLActivity; 

// Required to Override main
import android.os.Bundle;
// Required to add a dialog
import android.app.AlertDialog;
import android.content.DialogInterface;
// Required to go to a website
import android.content.Intent;
import android.net.Uri;

/* 
 * A sample wrapper class that just calls SDLActivity 
 */ 

public class BugSquasher extends SDLActivity {
	
	AlertDialog.Builder webDiagBuilder;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		webDiagBuilder = null;
	}

	public void gotoGFMGameCorner() {
		if (webDiagBuilder == null) {
			webDiagBuilder = new AlertDialog.Builder(this);
			webDiagBuilder.setTitle("Open author's website?");
			webDiagBuilder.setMessage("Go to http://gfmgamecorner.wordpress.com?");
			webDiagBuilder.setPositiveButton("Go", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
					String url = "http://gfmgamecorner.wordpress.com";
					Intent i = new Intent(Intent.ACTION_VIEW);
					i.setData(Uri.parse(url));
					startActivity(i);
				}
			});
			webDiagBuilder.setNegativeButton("Cancel", null);
		}
		runOnUiThread(new Runnable(){
			public void run() {
				AlertDialog dialog = webDiagBuilder.create();
				dialog.show();
			}
		});
	}
}

