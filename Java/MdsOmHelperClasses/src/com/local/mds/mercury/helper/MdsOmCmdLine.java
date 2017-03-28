package com.jpmorgan.mds.mercury.helper;

import java.util.List;

import com.wombat.mama.MamaBoolean;
import com.wombat.mama.MamaChar;
import com.wombat.mama.MamaInteger;
import com.wombat.mama.MamaLong;
import com.wombat.mama.MamaString;

public class MdsOmCmdLine {
	public MamaString argv;
	public MdsOmCmdLineType type;
	public MamaString desc;
	public MamaBoolean boolObj;
	public MamaChar charObj;
	public MamaString stringObj;
	public List<String> listObj;
	public MamaInteger intObj;
	public MamaLong longObj;
	
	// Command line processing
	public static enum MdsOmCmdLineType {
		BOOL,
		STRING,
		INT,
		LONG,
		CHAR,
		LIST,
		LIST_PARAM,
		HELP,
		NULL
	}
	
	// Process cmd line args using MdsOmCmdLine args array
	public static boolean processLineArgs(String argv[], MdsOmCmdLine args[])
	{
		// Duplicate MdsOmCmdLineType.LIST types not allowed as it is a 1-1 relationship to variables.
		if (!checkArgs(args)) return false;
		
		for (int x = 0; x < argv.length; ++x) {				// loop over cmd line args
			boolean setVar = false;
			for (int i = 0; i < args.length; ++i) {		// loop over program args and compare
				switch (args[i].type) {
					case BOOL:
						// match arg name and provided value
						if (args[i].argv.getValue().equals(argv[x]) && (x+1 < argv.length)) {
							if      (argv[x+1].equals("true")) args[i].boolObj.setValue(true);
							else if (argv[x+1].equals("false")) args[i].boolObj.setValue(false);
							else if (argv[x+1].equals("on")) args[i].boolObj.setValue(true);
							else if (argv[x+1].equals("off")) args[i].boolObj.setValue(false);
							else if (argv[x+1].equals("1")) args[i].boolObj.setValue(true);
							else if (argv[x+1].equals("0")) args[i].boolObj.setValue(false);
							else if (argv[x+1].equals("yes")) args[i].boolObj.setValue(true);
							else if (argv[x+1].equals("no")) args[i].boolObj.setValue(false);
							else if (!argv[x+1].equals("-")) {				//	is not a flag
								reportError(args,"Invalid bool entry",argv[x+1]);
								return false;
							} else {
								reportError(args,"Invalid bool argument",argv[x+1]);
								return false;
							}
							x++;
							setVar = true;
						}
						break;
					case STRING:
						// match arg name and provided value
						if (args[i].argv.getValue().equals(argv[x]) && (x+1 < argv.length)) {
							if (!argv[x+1].startsWith("-")){//is not a flag
								args[i].stringObj.setValue(argv[x+1]);
								x++;
								setVar=true;
							}
							else {
								reportError(args,"Invalid string argument",argv[x+1]);
								return false;
							}
						}
						break;
					case CHAR:
						// match arg name and provided value
						if(args[i].argv.getValue().equals(argv[x]) && (x+1 < argv.length)){
							if (!argv[x+1].startsWith("-")){	//is not a flag
								args[i].charObj.setValue(argv[x+1].charAt(0));
								x++;
								setVar=true;
							}
							else {
								reportError(args,"Invalid string argument",argv[x+1]);
								return false;
							}
						}
						break;
					case INT:
						//match arg name and provided value
						if(args[i].argv.getValue().equals(argv[x]) && (x+1 < argv.length)){
							if(!argv[x+1].startsWith("-")){//is not a flag
								args[i].intObj.setValue(Integer.parseInt(argv[x+1]));
								x++;
								setVar=true;
							}
							else {
								reportError(args,"Invalid int argument",argv[x+1]);
								return false;
							}
						}
						break;
					case LONG:
						//match arg name and provided value
						if(args[i].argv.getValue().equals(argv[x]) && (x+1 < argv.length)){
							if(!argv[x+1].startsWith("-")){//is not a flag
								args[i].longObj.setValue(Long.parseLong(argv[x+1]));
								x++;
								setVar=true;
							}
							else {
								reportError(args,"Invalid long argument",argv[x+1]);
								return false;
							}
						}
						break;
					case HELP:
						//match arg name
						if(args[i].argv.getValue().equals(argv[x])){
							reportError(args,"",argv[x]);
							return false;
						}
						break;
					case LIST_PARAM: 
						//if arg does not start with '-'
						if(args[i].argv.getValue().equals(argv[x]) && (x+1 < argv.length)){
							String a = argv[x+1];
							if(a.length() > 0 && !a.startsWith("-")){//is not a flag
								// Check to see if syms are comma seperated
								if(a.indexOf(',') != -1){
									String seps = ",";
									String[] tokens = a.split(seps);
									// Iterate over tokenized syms & add to queue
									for(int y=0; y<tokens.length; y++){
										args[i].listObj.add(new String(tokens[y]));
									}
								}else
									args[i].listObj.add(new String(argv[x+1]));
								x++;
								setVar=true;
							}
							else{
								reportError(args,"Invalid param argument", argv[x+1]);
								return false;
							}
						}
						break;
					case LIST: 
						//if arg does not start with '-'
						String a = argv[x];
						if(a.length() > 0 && !a.startsWith("-")){//is not a flag
							args[i].listObj.add(new String(argv[x]));
							setVar=true;
						}
						break;
					default:
						printUsage(args);
						return false;
				}
				if (setVar == true) break;//break out of while loop
			}
			if (setVar == false) {//unknown arg
				reportError(args,"Unknown argument",argv[x]);
				return false;
			}
		}
		
		return true;
	}

	// Print error for cmd line processing
	public static void reportError(MdsOmCmdLine args[], String desc, String argv){
		System.out.printf("Error: %s (%s)\n", desc, argv);
		printUsage(args);
	}

	// Print usage for cmd line processing
	public static void printUsage(MdsOmCmdLine args[]){
		int i=0;
		
		System.out.printf("Usage: [arg] desc [arg type]\n");
		while(i < args.length){
			String msg = "?";
			switch(args[i].type)
			{
				case BOOL:  		msg = "bool"; break;
				case STRING:		msg = "string"; break;
				case CHAR:			msg = "char"; break;
				case INT:    		msg = "int"; break;
				case LIST:  		msg = "list"; break;
				case LIST_PARAM:	msg = "list w/ param"; break;
				case HELP:   		msg = "help"; break;
				case NULL:   		msg = "null"; break;
				default:			msg = "unknown"; break;
			}
			System.out.printf("[%s] %s (%s)\n", args[i].argv, args[i].desc, msg);
			i++;
			
		}
	}
	public static boolean checkArgs(MdsOmCmdLine args[]){
		boolean listType=false;
		
		for(int i=0; i < args.length; i++){
			if(args[i].type.equals("MdsOmCmdLineType.LIST")){
				if(listType==true){
					reportError(args,"Cannot have more than one MdsOmCmdLineType.LIST type defined",args[i].type.toString());
					return false;
				}
				else
					listType=true;
			}
		}
		return true;
	}
	
	public MdsOmCmdLine(){
		this.argv = new MamaString("");
		this.type = MdsOmCmdLineType.NULL;
		this.desc = new MamaString("");
	}

	public MdsOmCmdLine(String argv, MdsOmCmdLineType type, String desc){
		this.argv = new MamaString(argv);
		this.type = type;
		this.desc = new MamaString(desc);	
	}
	
	public MdsOmCmdLine(String argv, MdsOmCmdLineType type, MamaBoolean obj, String desc){
		this.argv = new MamaString(argv);
		this.type = type;
		this.boolObj = obj;
		this.desc = new MamaString(desc);	
	}
		
	public MdsOmCmdLine(String argv, MdsOmCmdLineType type, MamaString obj, String desc){
		this.argv = new MamaString(argv);
		this.type = type;
		this.stringObj = obj;
		this.desc = new MamaString(desc);	
	}
	
	public MdsOmCmdLine(String argv, MdsOmCmdLineType type, MamaChar obj, String desc){
		this.argv = new MamaString(argv);
		this.type = type;
		this.charObj = obj;
		this.desc = new MamaString(desc);	
	}
	
	public MdsOmCmdLine(String argv, MdsOmCmdLineType type, List<String> obj, String desc){
		this.argv = new MamaString(argv);
		this.type = type;
		this.listObj = obj;
		this.desc = new MamaString(desc);	
	}
	
	public MdsOmCmdLine(String argv, MdsOmCmdLineType type, MamaInteger obj, String desc){
		this.argv = new MamaString(argv);
		this.type = type;
		this.intObj = obj;
		this.desc = new MamaString(desc);	
	}
	
	public MdsOmCmdLine(String argv, MdsOmCmdLineType type, MamaLong obj, String desc){
		this.argv = new MamaString(argv);
		this.type = type;
		this.longObj = obj;
		this.desc = new MamaString(desc);	
	}
} 
