package com.jpmorgan.mds.mercury.cmdline;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

import com.jpmorgan.mds.mercury.cmdline.Options.MdsOmCmdLineType;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.FIELD) //on class level
public @interface CmdLineOption {
	MdsOmCmdLineType type() default MdsOmCmdLineType.NULL;
	
	String description();
	
	String flag() default "";
	
	boolean required() default false;
}