package com.jpmorgan.mds.mercury.jms;

import com.wombat.mama.MamaPrice;
import com.wombat.mama.MamaPricePrecision;

public class MdsOmJmsPrice {
	private double val = 0.0;
	private int precision = 6;
	private boolean valid = true;
	
	public boolean isValid() {
		return valid;
	}
	
	public void setValid(boolean valid) {
		this.valid = valid;
	}
	
	public double getVal() {
		return val;
	}
	
	public void setVal(double val) {
		this.val = val;
	}
	
	public int getPrecision() {
		return precision;
	}
	
	public void setPrecision(int precision) {
		if (precision < 0) precision = 0;
		this.precision = precision;
	}
	
	private int precision2Decimals(MamaPricePrecision prec) {
		// TODO make this an OM patch
		if (prec == MamaPricePrecision.PRECISION_INT) return 0;
		if (prec == MamaPricePrecision.PRECISION_10) return 1;
		if (prec == MamaPricePrecision.PRECISION_100) return 2;
		if (prec == MamaPricePrecision.PRECISION_1000) return 3;
		if (prec == MamaPricePrecision.PRECISION_10000) return 4;
		if (prec == MamaPricePrecision.PRECISION_100000) return 5;
		if (prec == MamaPricePrecision.PRECISION_1000000) return 6;
		if (prec == MamaPricePrecision.PRECISION_10000000) return 7;
		if (prec == MamaPricePrecision.PRECISION_100000000) return 8;
		if (prec == MamaPricePrecision.PRECISION_1000000000) return 9;
		if (prec == MamaPricePrecision.PRECISION_10000000000) return 10;
		return 10;
	}
	
	public void setFromMamaPrice(MamaPrice mp) {
		val = mp.getValue();
		valid = mp.getIsValidPrice();
		precision = precision2Decimals(mp.getPrecision());		
	}
	
	public void setToMamaPrice(MamaPrice mp) {
		mp.setValue(val);
		mp.setIsValidPrice(valid);
		mp.setPrecision(MamaPricePrecision.decimals2Precision(precision));		
	}
	
	public String toString() {
		return String.valueOf(val);
	}
}
