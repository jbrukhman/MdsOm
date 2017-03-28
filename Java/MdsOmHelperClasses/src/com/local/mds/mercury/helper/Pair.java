package com.jpmorgan.mds.mercury.helper;

public class Pair<x, y> {
	public x x;
	public y y;
	
	public Pair(x x, y y) {
		this.x = x;
		this.y = y;
	}
	
	public x getX() {
		return x;
	}
	
	public y getY() {
		return y;
	}
	
	public void setX(x x) {
		this.x = x;
	}
	
	public void setY(y y) {
		this.y = y;
	}
}
