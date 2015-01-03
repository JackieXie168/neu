/*

      ___           ___           ___
     /\__\         /\  \         /\__\
    /::|  |       /::\  \       /:/  /
   /:|:|  |      /:/\:\  \     /:/  /
  /:/|:|  |__   /::\~\:\  \   /:/  /  ___
 /:/ |:| /\__\ /:/\:\ \:\__\ /:/__/  /\__\
 \/__|:|/:/  / \:\~\:\ \/__/ \:\  \ /:/  /
     |:/:/  /   \:\ \:\__\    \:\  /:/  /
     |::/  /     \:\ \/__/     \:\/:/  /
     /:/  /       \:\__\        \::/  /
     \/__/         \/__/         \/__/


The Neu Framework, Copyright (c) 2013-2015, Andrometa LLC
All rights reserved.

neu@andrometa.net
http://neu.andrometa.net

Neu can be used freely for commercial purposes. If you find Neu
useful, please consider helping to support our work and the evolution
of Neu by making a donation via: http://donate.andrometa.net

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
 
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
 
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
*/

package net.andrometa.neu;

import java.util.ArrayList;
import java.util.TreeMap;
import java.util.Map;

import java.io.IOException;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.io.DataOutputStream;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.zip.Deflater;
import java.util.zip.Inflater;
import java.util.zip.DataFormatException;
import java.io.FileOutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.ByteBuffer;
import java.nio.file.Paths;
import java.nio.file.Files;

import net.andrometa.neu.NError;
import net.andrometa.neu.nrat;
import net.andrometa.neu.nreal;

public class nvar implements Comparable<nvar>{
	public static final char None =                     5;
	public static final char Undefined =                6;
	public static final char False =                    7;
	public static final char True =                     8;
	public static final char Integer =                  9;
	public static final char Rational =                10;
	public static final char Float =                   11;
	public static final char Real =                    12;
	public static final char Symbol =                  13;
	public static final char String =                  14;
	public static final char Binary =                  16;
	public static final char Vector =                  21;
	public static final char List =                    22;
	public static final char Queue =                   23;
	public static final char Function =                24;
	public static final char HeadSequence =            25;
	public static final char Set =                     26;
	public static final char HashSet =                 27;
	public static final char Map =                     28;
	public static final char HashMap =                 29;
	public static final char Multimap =                30;
	public static final char HeadMap =                 31;
	public static final char SequenceMap =             32;
	public static final char HeadSequenceMap =         33;

  public static final char Pack0 =                  254;
  public static final char Pack1 =                  253;
  public static final char Pack2 =                  252;
  public static final char Pack3 =                  251;
  public static final char Pack4 =                  250;
  public static final char Pack5 =                  249;
  public static final char Pack6 =                  248;
  public static final char Pack7 =                  247;
  public static final char Pack8 =                  246;
  public static final char Pack9 =                  245;
  public static final char Pack10 =                 244;
  public static final char Pack11 =                 243;
  public static final char Pack12 =                 242;
  public static final char Pack13 =                 241;
  public static final char Pack14 =                 240;
  public static final char Pack15 =                 239;
  public static final char Pack16 =                 238;
  public static final char Pack17 =                 237;
  public static final char Pack18 =                 236;
  public static final char Pack19 =                 235;
  public static final char Pack20 =                 234;
  public static final char Pack21 =                 233;
  public static final char Pack22 =                 232;
  public static final char Pack23 =                 231;
  public static final char Pack24 =                 230;
  public static final char Pack25 =                 229;
  public static final char Pack26 =                 228;
  public static final char Pack27 =                 227;
  public static final char Pack28 =                 226;
  public static final char Pack29 =                 225;
  public static final char Pack30 =                 224;
  public static final char Pack31 =                 223;
  public static final char Pack32 =                 222;
  public static final char Pack33 =                 221;
  public static final char Pack34 =                 220;
  public static final char Pack35 =                 219;
  public static final char Pack36 =                 218;
  public static final char Pack37 =                 217;
  public static final char Pack38 =                 216;
  public static final char Pack39 =                 215;
  public static final char Pack40 =                 214;
  public static final char Pack41 =                 213;
  public static final char Pack42 =                 212;
  public static final char Pack43 =                 211;
  public static final char Pack44 =                 210;
  public static final char Pack45 =                 209;
  public static final char Pack46 =                 208;
  public static final char Pack47 =                 207;
  public static final char Pack48 =                 206;
  public static final char Pack49 =                 205;
  public static final char Pack50 =                 204;
  public static final char Pack51 =                 203;
  public static final char Pack52 =                 202;
  public static final char Pack53 =                 201;
  public static final char Pack54 =                 200;
  public static final char Pack55 =                 199;
  public static final char Pack56 =                 198;
  public static final char Pack57 =                 197;
  public static final char Pack58 =                 196;
  public static final char Pack59 =                 195;
  public static final char Pack60 =                 194;
  public static final char Pack61 =                 193;
  public static final char Pack62 =                 192;
  public static final char Pack63 =                 191;
  public static final char Pack64 =                 190;
  public static final char Pack65 =                 189;
  public static final char Pack66 =                 188;
  public static final char Pack67 =                 187;
  public static final char Pack68 =                 186;
  public static final char Pack69 =                 185;
  public static final char Pack70 =                 184;
  public static final char Pack71 =                 183;
  public static final char Pack72 =                 182;
  public static final char Pack73 =                 181;
  public static final char Pack74 =                 180;
  public static final char Pack75 =                 179;
  public static final char Pack76 =                 178;
  public static final char Pack77 =                 177;
  public static final char Pack78 =                 176;
  public static final char Pack79 =                 175;
  public static final char Pack80 =                 174;
  public static final char Pack81 =                 173;
  public static final char Pack82 =                 172;
  public static final char Pack83 =                 171;
  public static final char Pack84 =                 170;
  public static final char Pack85 =                 169;
  public static final char Pack86 =                 168;
  public static final char Pack87 =                 167;
  public static final char Pack88 =                 166;
  public static final char Pack89 =                 165;
  public static final char Pack90 =                 164;
  public static final char Pack91 =                 163;
  public static final char Pack92 =                 162;
  public static final char Pack93 =                 161;
  public static final char Pack94 =                 160;
  public static final char Pack95 =                 159;
  public static final char Pack96 =                 158;
  public static final char Pack97 =                 157;
  public static final char Pack98 =                 156;
  public static final char Pack99 =                 155;
  public static final char Pack100 =                154;
  public static final char Pack101 =                153;
  public static final char Pack102 =                152;
  public static final char Pack103 =                151;
  public static final char Pack104 =                150;
  public static final char Pack105 =                149;
  public static final char Pack106 =                148;
  public static final char Pack107 =                147;
  public static final char Pack108 =                146;
  public static final char Pack109 =                145;
  public static final char Pack110 =                144;
  public static final char Pack111 =                143;
  public static final char Pack112 =                142;
  public static final char Pack113 =                141;
  public static final char Pack114 =                140;
  public static final char Pack115 =                139;
  public static final char Pack116 =                138;
  public static final char Pack117 =                137;
  public static final char Pack118 =                136;
  public static final char Pack119 =                135;
  public static final char Pack120 =                134;
  public static final char Pack121 =                133;
  public static final char Pack122 =                132;
  public static final char Pack123 =                131;
  public static final char Pack124 =                130;
  public static final char Pack125 =                129;
  public static final char Pack126 =                128;
  public static final char Pack127 =                127;
  public static final char PackInt8 =               126;
  public static final char PackInt16 =              125;
  public static final char PackInt32 =              124;
  public static final char PackFloat32 =            123;
  public static final char PackShortString =        122;
  public static final char PackLongString =         121;
  public static final char PackLongSymbol =         120;
  public static final char PackShortVector =        118;
  public static final char PackLongVector =         117;
  public static final char PackShortList =          116;
  public static final char PackLongList =           115;
  public static final char PackShortQueue =         114;
  public static final char PackLongQueue =          113;
  public static final char PackLongFunction =       112;
  public static final char PackShortSet =           111;
  public static final char PackLongSet =            110;
  public static final char PackShortHashSet =       109;
  public static final char PackLongHashSet =        108;
  public static final char PackShortMap =           107;
  public static final char PackLongMap =            106;
  public static final char PackShortHashMap =       105;
  public static final char PackLongHashMap =        104;
  public static final char PackShortMultimap =      103;
  public static final char PackLongMultimap =       102;

	private class Map_{
		public Map_(char type){
			type_ = type;
			map_ = new TreeMap<nvar, nvar>();
		}

		public nvar get(nvar k){
			return map_.get(k);
		}		

		public void put(nvar k, nvar v){
			map_.put(k, v);
		}

		public int size(){
			return map_.size();
		}

		public char type(){
			return type_;
		}

		public void setType(char type){
			type_ = type;
		}

		public boolean containsKey(nvar k){
			return map_.containsKey(k);
		}

		public void keys(ArrayList<nvar> keys){
			for(Map.Entry<nvar, nvar> entry : map_.entrySet()){
				keys.add(entry.getKey());
			}
		}

		public boolean isEmpty(){
			return map_.isEmpty();
		}

		public void remove(nvar k){
			map_.remove(k);
		}

		public String toStr(){
			String str = new String();
			
			boolean first = true;

			for(Map.Entry<nvar, nvar> entry : map_.entrySet()){
				if(first){
					first = false;
				}
				else{
					str += ", ";
				}
				str += entry.getKey() + ":" + entry.getValue();
			}

			return str;
		}

		public String token_(){
			switch(type_){
			case Set:
				return "% ";
			case HashSet:
				return "^";
			case Map:
				return "";
			case HashMap:
				return "= ";
			case Multimap:
				return "| ";
			default:
				assert false;
			}

			return "";
		}

		private void pack_(DataOutputStream dstr) throws Exception{
			switch(type_){
			case Set:
				dstr.writeByte(PackLongSet);
				break;
			case HashSet:
				dstr.writeByte(PackLongHashSet);
				break;
			case Map:
				dstr.writeByte(PackLongMap);
				break;
			case HashMap:
				dstr.writeByte(PackLongHashMap);
				break;
			case Multimap:
				dstr.writeByte(PackLongMultimap);
				break;
			default:
				assert false;
			}

			int size = map_.size();
			dstr.writeInt(java.lang.Integer.reverseBytes(size));

			if(type_ == Set || type_ == HashSet){
				for(Map.Entry<nvar, nvar> entry : map_.entrySet()){
					entry.getKey().pack_(dstr);
				}
			}
			else{
				for(Map.Entry<nvar, nvar> entry : map_.entrySet()){
					entry.getKey().pack_(dstr);
					entry.getValue().pack_(dstr);
				}
			}
		}

		private void unpack_(DataInputStream dstr, int size) throws Exception{
			if(type_ == Set || type_ == HashSet){
				for(int i = 0; i < size; ++i){
					nvar ki = new nvar();
					ki.unpack_(dstr);

					map_.put(ki, new nvar(true));
				}
			}
			else{
				for(int i = 0; i < size; ++i){
					nvar ki = new nvar();
					ki.unpack_(dstr);

					nvar vi = new nvar();
					vi.unpack_(dstr);

					map_.put(ki, vi);
				}
			}
		}

		private char type_;
		private TreeMap<nvar, nvar> map_;
	}

	private class Sequence_{
		public Sequence_(char type){
			type_ = type;
			sequence_ = new ArrayList<nvar>();
		}

		public int size(){
			return sequence_.size();
		}

		public void add(int i, nvar v){
			sequence_.add(i, v);
		}

		public void add(nvar v){
			sequence_.add(v);
		}

		public nvar get(int i){
			return sequence_.get(i);
		}

		public void set(int i, nvar v){
			sequence_.set(i, v);
		}

		public boolean isEmpty(){
			return sequence_.isEmpty();
		}

		public char type(){
			return type_;
		}

		public void setType(char type){
			type_ = type;
		}

		public String toStr(){
			String str = new String();
			
			boolean first = true;

			for(nvar vi : sequence_){
				if(first){
					first = false;
				}
				else{
					str += ",";
				}
				str += vi.toString();
			}

			return str;
		}

		public String token_(){
			switch(type_){
			case Vector:
				return "";
			case List:
				return "`";
			case Queue:
				return "@";
			default:
				assert false;
			}

			return "";
		}

		private void pack_(DataOutputStream dstr) throws Exception{
			switch(type_){
			case Vector:
				dstr.writeByte(PackLongVector);
				break;
			case List:
				dstr.writeByte(PackLongList);
				break;
			case Queue:
				dstr.writeByte(PackLongQueue);
				break;
			default:
				assert false;
			}

			int size = sequence_.size();
			dstr.writeInt(java.lang.Integer.reverseBytes(size));
		
			for(int i = 0; i < size; ++i){
				nvar v = sequence_.get(i);
				v.pack_(dstr);
			}
		}

		private void unpack_(DataInputStream dstr, int size) throws Exception{
			for(int i = 0; i < size; ++i){
				nvar vi = new nvar();
				vi.unpack_(dstr);
				add(vi);
			}
		}		

		private char type_;
		private ArrayList<nvar> sequence_;
	}

	public nvar(){
		type_ = Undefined;
		head_ = null;
		
		sequence_ = null;
		map_ = null;
	}

	public nvar(boolean x){
		type_ = x ? True : False;
		head_ = new Boolean(x);
		
		sequence_ = null;
		map_ = null;
	}

	public nvar(long x){
		type_ = Integer;
		head_ = new Long(x);

		sequence_ = null;
		map_ = null;
	}

	public nvar(double x){
		type_ = Float;
		head_ = new Double(x);

		sequence_ = null;
		map_ = null;
	}

	public nvar(nrat x){
		type_ = Rational;
		head_ = x;

		sequence_ = null;
		map_ = null;
	}

	public nvar(nreal x){
		type_ = Real;
		head_ = x;

		sequence_ = null;
		map_ = null;
	}

	public nvar(String x){
		type_ = String;
		head_ = x;

		sequence_ = null;
		map_ = null;
	}

	public nvar(byte[] buf) throws Exception{
		type_ = Undefined;
		head_ = null;
		
		sequence_ = null;
		map_ = null;

		unpack(buf);
	}

	public static nvar func(String f){
		nvar n = new nvar(f);
		n.type_ = Function;

		n.intoVector();
		n.intoMap();

		return n;
	}

	public static nvar sym(String s){
		nvar n = new nvar(s);
		n.type_ = Symbol;

		return n;
	}

	public static nvar undef(){
		return new nvar();
	}

	public int size(){
		if(sequence_ == null){
			return 0;
		}

		return sequence_.size();
	}

	public void add(nvar x){
		if(sequence_ == null){
			sequence_ = new Sequence_(Vector);
		}

		sequence_.add(x);
	}

	public void add(long x){
		add(new nvar(x));
	}

	public void add(double x){
		add(new nvar(x));
	}

	public void add(String x){
		add(new nvar(x));
	}

	public nvar get(int i){
		if(sequence_ == null){
			throw new NError("invalid index: " + i);
		}

		try{
			return sequence_.get(i);
		}
		catch(Exception e){
			throw new NError("invalid index: " + i);
		}
	}

	public long getLong(int i){
		return get(i).asLong();
	}

	public int getInt(int i){
		return (int)get(i).asLong();
	}

	public double getDouble(int i){
		return get(i).asDouble();
	}

	public float getFloat(int i){
		return (float)get(i).asDouble();
	}

	public String getStr(int i){
		return get(i).str();
	}

	public nvar get(nvar k){
		if(map_ == null){
			throw new NError("invalid key: " + k);
		}

		try{
			return map_.get(k);
		}
		catch(Exception e){
			throw new NError("invalid key: " + k);
		}
	}

	public long getLong(nvar k){
		return get(k).asLong();
	}

	public int getInt(nvar k){
		return (int)get(k).asLong();
	}

	public double getDouble(nvar k){
		return get(k).asDouble();
	}

	public float getFloat(nvar k){
		return (float)get(k).asDouble();
	}

	public String getStr(nvar k){
		return get(k).str();
	}

	static boolean isSymbol(String s){
		int len = s.length();

		boolean past = false;
		for(int i = 0; i < len; ++i){
			char c = s.charAt(i);

			if(Character.isLetter(c)){
				past = true; 
			}
			else if(Character.isDigit(c)){
				if(!past){
					return false;
				}
			}
			else if(c != '_'){
				return false;
			}
		}

		return true;
	}

	static nvar toKey(String k){
		if(isSymbol(k)){
			return sym(k);
		}

		return new nvar(k);
	}

	public nvar put(nvar k, nvar v){
		if(map_ == null){
			map_ = new Map_(Map);
		}

		map_.put(k, v);

		return this;
	}

	public nvar put(nvar k, long x){
		return put(k, new nvar(x));
	}

	public nvar put(nvar k, double x){
		return put(k, new nvar(x));
	}

	public nvar put(nvar k, String x){
		return put(k, new nvar(x));
	}

	public nvar put(String k, nvar v){
		return put(toKey(k), v);
	}

	public nvar put(String k, long x){
		return put(toKey(k), new nvar(x));
	}

	public nvar put(String k, double x){
		return put(toKey(k), new nvar(x));
	}

	public nvar put(String k, String x){
		return put(toKey(k), new nvar(x));
	}

	public nvar add(int index, nvar v){
		if(sequence_ == null){
			sequence_ = new Sequence_(Vector);
		}

		while(index >= sequence_.size()){
			sequence_.add(new nvar());
		}

		sequence_.set(index, v);
		
		return this;
	}

	public nvar add(int index, long x){
		return add(index, new nvar(x));
	}

	public nvar add(int index, double x){
		return add(index, new nvar(x));
	}

	public nvar add(int index, String x){
		return add(index, new nvar(x));
	}

	public char type(){
		switch(type_){
		case HeadSequence:
		case HeadMap:
		case HeadSequenceMap:
			break;
		default:
			return type_;
		}

		if(head_ instanceof Long){
			return Integer;
		}

		if(head_ instanceof Double){
			return Float;
		}

		if(head_ instanceof String){
			return String;
		}

		if(head_ instanceof Boolean){
			return (Boolean)head_ ? True : False;
		}

		if(head_ instanceof nrat){
			return Rational;
		}

		if(head_ instanceof nreal){
			return Real;
		}
			
		return Undefined;
	}

	public char fullType(){
		if(sequence_ != null){
			if(map_ != null){
				switch(type_){
				case Undefined:
					return SequenceMap;
				case Function:
					return Function;
				default:
					return HeadSequenceMap;
				}
			}
			else{
				switch(type_){
				case Undefined:
					return sequence_.type();
				default:
					return HeadSequence;
				}
			}
		}
		else if(map_ != null){
			switch(type_){
			case Undefined:
				return map_.type();
			default:
				return HeadMap;
			}
		}
		else{
			return type_;
		}
	}

	public boolean isString(){
		return type_ == String;
	}

	public boolean isNumeric(){
		switch(type_){
		case False:
		case True:
		case Integer:
		case Rational:
		case Float:
		case Real:
			return true;
		default:
			return false;
		}
	}

	public boolean isTrue(){
		return type_ == True;
	}

	public boolean isFalse(){
		return type_ == False;
	}

	public double toDouble(){
		switch(type_){
		case Float:
			return (Double)head_;
		case Rational:{
			nrat r = (nrat)head_;
			return (double)r.numerator/r.denominator;
		}
		case Integer:
			return (Long)head_;
		case String:{
			try{
				return Double.parseDouble((String)head_);
			}
			catch(Exception e){
				throw new NError("invalid conversion to double"); 
			}
		}
		}
		
		throw new NError("invalid conversion to double"); 
	}

	public float toFloat(){
		return (float)toDouble();
	}

	public double asDouble(){
		if(type_ != Float){
			throw new NError("not a double");
		}

		return (Double)head_;
	}

	public long toLong(){
		switch(type_){
		case Float:
			return (long)((Double)head_).doubleValue();
		case Integer:
			return (Long)head_;
		case String:{
			try{
				return (long)Double.parseDouble((String)head_);
			}
			catch(Exception e){
				throw new NError("invalid conversion to long"); 
			}
		}
		}

		throw new NError("invalid conversion to long"); 
	}

	public int toInt(){
		return (int)toLong();
	}

	public long asLong(){
		if(type_ != Integer){
			throw new NError("not a long");
		}

		return (Long)head_;
	}

	public nrat asRat(){
		if(type_ != Rational){
			throw new NError("not a rational");
		}

		return (nrat)head_;
	}

	public nreal asReal(){
		if(type_ != Rational){
			throw new NError("not a real");
		}

		return (nreal)head_;
	}

	public boolean toBool(){
		switch(type_){
		case Float:
			return (Double)head_ != 0.0;
		case False:
			return false;
		case True:
			return true;	
		case Integer:{
			return (Long)head_ != 0;
		}
		case String:{
			try{
				return Double.parseDouble((String)head_) != 0.0;
			}
			catch(Exception e){
				throw new NError("invalid conversion to bool"); 
			}
		}
		}

		throw new NError("invalid conversion to bool"); 
	}

	public static nvar fromStr(String str){
		try{
			return new nvar(Long.parseLong(str));
		}
		catch(NumberFormatException e){}

		try{
			return new nvar(Double.parseDouble(str));
		}
		catch(NumberFormatException e){}

		return new nvar(str);
	}
	
	public String str(){
		switch(type_){
		case String:
		case Symbol:
		case Binary:
		case Function:
			return (String)head_;
		}

		throw new NError("var does not hold a string");
	}

	private String toStr_(String str){
		switch(fullType()){
		case None:
			return "none";
		case Undefined:
			return "undef";
		case False:
			return "false";	
		case True:
			return "true";
		case Integer:
		case Float:
		case Symbol:
			return head_.toString();
		case String:
			return "\"" + head_ + "\"";
		case Binary:
			return "<<BINARY:" + ((String)head_).length() + ">>";
		case Vector:
		case List:
		case Queue:
			return sequence_.token_() + "[" + sequence_.toStr() + "]";
		case Function:
			return (String)head_ + "(" + sequence_.toStr() + ")";
		case Set:
		case HashSet:
		case Map:
		case HashMap:
		case Multimap:
			return "[" + map_.token_() + map_.toStr() + "]";
		case HeadSequence:{
			String s = sequence_.token_() + "[:" + head_.toString();
			
			if(!sequence_.isEmpty()){
				s += ", " + sequence_.toStr();
			}

			s += "]";

			return s;
		}
		case HeadMap:{
			String s = "[" + map_.token_() + ":" + head_.toString();
			
			if(!map_.isEmpty()){
				s += ", " + map_.toStr();
			}

			s += "]";

			return s;
		}
		case SequenceMap:{
			String s = sequence_.token_() + "[" + map_.token_();

			boolean hasMap = !map_.isEmpty();

			if(!sequence_.isEmpty()){
				s += " " + sequence_.toStr();

				if(hasMap){
					s += ", ";
				}
			}

			if(hasMap){
				s += map_.toStr();
			}

			s += "]";

			return s;
		}
		case HeadSequenceMap:{
			String s = sequence_.token_() + "[" + map_.token_() + ":" + head_.toString();
			
			boolean hasMap = !map_.isEmpty();

			if(!sequence_.isEmpty()){
				s += " " + sequence_.toStr();

				if(hasMap){
					s += ", ";
				}
			}

			if(hasMap){
				s += map_.toStr();
			}

			s += "]";

			return s;
		}
		}

		return "";
	}

	public String toString(){
		String str = new String();
		str = toStr_(str);
		return str;
	}

	public boolean equals(Object obj){
		if(obj instanceof nvar){
			return toString() == obj.toString();
		}
		
		if(obj instanceof Long){
			return toLong() == (Long)obj;
		}

		if(obj instanceof Double){
			return toDouble() == (Double)obj;
		}

		if(obj instanceof String){
			return str() == (String)obj;
		}

		if(obj instanceof Boolean){
			return toBool() == (Boolean)obj;
		}

		return false;
	}

	public int compareTo(nvar v){
		return toString().compareTo(v.toString());
	}

	public boolean isUndef(){
		return fullType() == Undefined;
	}

	public boolean isDefined(){
		return type_ != Undefined;
	}

	public boolean isEmpty(){
		return sequence_ == null || sequence_.isEmpty();
	}

	public int numKeys(){
		return map_ == null ? 0 : map_.size();
	}

	public boolean hasSequence(){
		return sequence_ != null;
	}

	public boolean hasMap(){
		return map_ != null;
	}

	public boolean has(nvar k){
		return map_ == null ? false : map_.containsKey(k);
	}

	public ArrayList<nvar> keys(){
		ArrayList<nvar> ret = new ArrayList<nvar>();

		if(map_ == null){
			return ret;
		}

		map_.keys(ret);
		
		return ret;
	}

	public void intoVector(){
		if(sequence_ == null){
			sequence_ = new Sequence_(Vector);
		}
		else{
			sequence_.setType(Vector);
		}
	}

	public void intoList(){
		if(sequence_ == null){
			sequence_ = new Sequence_(List);
		}
		else{
			sequence_.setType(List);
		}
	}

	public void intoQueue(){
		if(sequence_ == null){
			sequence_ = new Sequence_(Queue);
		}
		else{
			sequence_.setType(Queue);
		}
	}

	public void intoSet(){
		if(map_ == null){
			map_ = new Map_(Set);
		}
		else{
			map_.setType(Set);
		}
	}

	public void intoHashSet(){
		if(map_ == null){
			map_ = new Map_(HashSet);
		}
		else{
			map_.setType(HashSet);
		}
	}

	public void intoHashMap(){
		if(map_ == null){
			map_ = new Map_(HashMap);
		}
		else{
			map_.setType(HashMap);
		}
	}

	public void intoMap(){
		if(map_ == null){
			map_ = new Map_(Map);
		}
		else{
			map_.setType(Map);
		}
	}

	public void intoMultimap(){
		if(map_ == null){
			map_ = new Map_(Multimap);
		}
		else{
			map_.setType(Multimap);
		}
	}

	public void erase(nvar k){
		if(map_ == null){
			return;
		}

		map_.remove(k);
	}

	public nvar head(){
		nvar h = new nvar();

		h.type_ = type();
		h.head_ = head_;

		return h;
	}

	public byte[] pack() throws Exception{
		ByteArrayOutputStream bstr = new ByteArrayOutputStream();
		DataOutputStream dstr = new DataOutputStream(bstr);

		dstr.writeByte(0);

		pack_(dstr);
		bstr.close();
		dstr.close();

		return bstr.toByteArray();
	}

	public void unpack(byte[] data) throws Exception{
		ByteArrayInputStream bstr = new ByteArrayInputStream(data);
		DataInputStream dstr = new DataInputStream(bstr);

		dstr.readUnsignedByte();

		unpack_(dstr);
		bstr.close();
		dstr.close();
	}

	public static double reverseBytes(double x){
		long l = Long.reverseBytes(Double.doubleToLongBits(x));
		return Double.longBitsToDouble(l);
	}

	public static float reverseBytes(float x){
		int i = java.lang.Integer.reverseBytes(java.lang.Float.floatToIntBits(x));
		return java.lang.Float.intBitsToFloat(i);
	}

	public static short reverseBytes(short x){
		return Short.reverseBytes(x);
	}

	public static int reverseBytes(int x){
		return java.lang.Integer.reverseBytes(x);
	}
	
	public static long reverseBytes(long x){
		return Long.reverseBytes(x);
	}

	private void pack_(DataOutputStream dstr) throws Exception{
		switch(fullType()){
		case None:
		case Undefined:
		case True:
    case False:
			dstr.writeByte(type_);
			break;
		case Integer:
			dstr.writeByte(type_);
			dstr.writeLong(reverseBytes(asLong()));
			break;
		case Rational:{
			dstr.writeByte(type_);
			nrat r = asRat();
			dstr.writeLong(reverseBytes(r.numerator));
			dstr.writeLong(reverseBytes(r.denominator));
			break;
		}
		case Float:
			dstr.writeByte(type_);
			dstr.writeDouble(reverseBytes(asDouble()));
			break;
		case Real:{
			dstr.writeByte(Real);
			nreal r = asReal();
			short len = (short)r.r.length();
			dstr.writeShort(reverseBytes(len));
			dstr.writeBytes(r.r);
			break;
		}
		case Symbol:{
			dstr.writeByte(PackLongSymbol);
			String str = (String)head_;

			int len = str.length();
			dstr.writeInt(reverseBytes(len));
			dstr.writeBytes(str);
			break;
		}
		case String:{
			dstr.writeByte(PackLongString);
			String str = (String)head_;

			int len = str.length();
			dstr.writeInt(reverseBytes(len));
			dstr.writeBytes(str);
			break;
		}
		case Binary:{
			dstr.writeByte(Binary);
			String str = (String)head_;

			int len = str.length();
			dstr.writeInt(reverseBytes(len));
			dstr.writeBytes(str);
			break;
		}
		case Vector:
		case List:
		case Queue:
			sequence_.pack_(dstr);
			break;
		case Function:{
			dstr.writeByte(PackLongFunction);
			String func = (String)head_;

			int len = func.length();
			dstr.writeInt(reverseBytes(len));
			dstr.writeBytes(func);

			sequence_.pack_(dstr);
			map_.pack_(dstr);
			break;
		}
		case HeadSequence:{
			dstr.writeByte(HeadSequence);
			nvar h = head();
			h.pack_(dstr);
			sequence_.pack_(dstr);
		}
		case Set:
		case HashSet:
		case Map:
		case HashMap:
		case Multimap:
			map_.pack_(dstr);
			break;
		case HeadMap:{
			dstr.writeByte(HeadMap);
			nvar h = head();
			h.pack_(dstr);
			map_.pack_(dstr);
			break;
		}
		case SequenceMap:{
			dstr.writeByte(SequenceMap);
			sequence_.pack_(dstr);
			map_.pack_(dstr);
			break;
		}
		case HeadSequenceMap:{
			dstr.writeByte(HeadSequenceMap);
			nvar h = head();
			h.pack_(dstr);
			sequence_.pack_(dstr);
			map_.pack_(dstr);
			break;
		}
		default:
			assert false;
		}
	}

	private void unpack_(DataInputStream dstr) throws Exception{
		char type = (char)dstr.readUnsignedByte();

		switch(type){
		case None:
		case Undefined:
    case False:
    case True:
			type_ = type;
			break;
		case Pack0:
    case Pack1:
    case Pack2:
    case Pack3:
    case Pack4:
    case Pack5:
    case Pack6:
    case Pack7:
    case Pack8:
    case Pack9:
    case Pack10:
    case Pack11:
    case Pack12:
    case Pack13:
    case Pack14:
    case Pack15:
    case Pack16:
    case Pack17:
    case Pack18:
    case Pack19:
    case Pack20:
    case Pack21:
    case Pack22:
    case Pack23:
    case Pack24:
    case Pack25:
    case Pack26:
    case Pack27:
    case Pack28:
    case Pack29:
    case Pack30:
    case Pack31:
    case Pack32:
    case Pack33:
    case Pack34:
    case Pack35:
    case Pack36:
    case Pack37:
    case Pack38:
    case Pack39:
    case Pack40:
    case Pack41:
    case Pack42:
    case Pack43:
    case Pack44:
    case Pack45:
    case Pack46:
    case Pack47:
    case Pack48:
    case Pack49:
    case Pack50:
    case Pack51:
    case Pack52:
    case Pack53:
    case Pack54:
    case Pack55:
    case Pack56:
    case Pack57:
    case Pack58:
    case Pack59:
    case Pack60:
    case Pack61:
    case Pack62:
    case Pack63:
    case Pack64:
    case Pack65:
    case Pack66:
    case Pack67:
    case Pack68:
    case Pack69:
    case Pack70:
    case Pack71:
    case Pack72:
    case Pack73:
    case Pack74:
    case Pack75:
    case Pack76:
    case Pack77:
    case Pack78:
    case Pack79:
    case Pack80:
    case Pack81:
    case Pack82:
    case Pack83:
    case Pack84:
    case Pack85:
    case Pack86:
    case Pack87:
    case Pack88:
    case Pack89:
    case Pack90:
    case Pack91:
    case Pack92:
    case Pack93:
    case Pack94:
    case Pack95:
    case Pack96:
    case Pack97:
    case Pack98:
    case Pack99:
    case Pack100:
    case Pack101:
    case Pack102:
    case Pack103:
    case Pack104:
    case Pack105:
    case Pack106:
    case Pack107:
    case Pack108:
    case Pack109:
    case Pack110:
    case Pack111:
    case Pack112:
    case Pack113:
    case Pack114:
    case Pack115:
    case Pack116:
    case Pack117:
    case Pack118:
    case Pack119:
    case Pack120:
    case Pack121:
    case Pack122:
    case Pack123:
    case Pack124:
    case Pack125:
    case Pack126:
    case Pack127:
      type_ = Integer;
      head_ = new Long(Pack0 - type);
      break;
    case PackInt8:
      type_ = Integer;
			head_ = new Long(dstr.readByte());
      break;
    case PackInt16:
      type_ = Integer;
			head_ = Long.reverseBytes(dstr.readShort());
      break;
    case PackInt32:
      type_ = Integer;
			head_ = Long.reverseBytes(dstr.readInt());
      break;
    case Integer:
      type_ = Integer;
			head_ = Long.reverseBytes(dstr.readLong());
      break;
    case Rational:{
      type_ = Rational;
			long n = Long.reverseBytes(dstr.readLong());
			long d = Long.reverseBytes(dstr.readLong());
			head_ = new nrat(n, d);
      break;
		}
    case PackFloat32:
      type_ = Float;
			head_ = new Double(reverseBytes(dstr.readFloat()));
      break;
    case Float:
      type_ = Float;
			head_ = reverseBytes(dstr.readDouble());
      break;
		case Real:{
			type_ = Real;
			int size = reverseBytes(dstr.readUnsignedShort());
			byte[] buf = new byte[size];
			int n = dstr.read(buf, 0, size);
			assert n == size;
			String s = new String(buf);
			head_ = new nreal(s);
			break;
		}
		case Symbol:{
			type_ = Symbol;
			int size = dstr.readUnsignedByte();
			byte[] buf = new byte[size];
			int n = dstr.read(buf, 0, size);
			assert n == size;
			head_ = new String(buf);
			break;
		}
		case PackLongSymbol:{
			type_ = Symbol;
			int size = reverseBytes(dstr.readInt());
			byte[] buf = new byte[size];
			int n = dstr.read(buf, 0, size);
			assert n == size;
			head_ = new String(buf);
			break;
		}
		case PackShortString:{
			type_ = String;
			int size = dstr.readUnsignedByte();
			byte[] buf = new byte[size];
			int n = dstr.read(buf, 0, size);
			assert n == size;
			head_ = new String(buf);
			break;
		}
		case String:{
			type_ = String;
			int size = reverseBytes(dstr.readUnsignedShort());
			byte[] buf = new byte[size];
			int n = dstr.read(buf, 0, size);
			assert n == size;
			head_ = new String(buf);
			break;
		}
		case PackLongString:{
			type_ = String;
			int size = reverseBytes(dstr.readInt());

			byte[] buf = new byte[size];

			int n = dstr.read(buf, 0, size);
			assert n == size;

			head_ = new String(buf);

			break;
		}
		case Binary:{
			type_ = Binary;
			int size = reverseBytes(dstr.readInt());
			byte[] buf = new byte[size];
			int n = dstr.read(buf, 0, size);
			assert n == size;
			head_ = new String(buf);
			break;
		}
		case PackShortVector:{
			type_ = Undefined;
			int size = dstr.readUnsignedByte();
			sequence_ = new Sequence_(Vector);
			sequence_.unpack_(dstr, size);
			break;
		}
		case Vector:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readUnsignedShort());
			sequence_ = new Sequence_(Vector);
			sequence_.unpack_(dstr, size);
			break;
		}
		case PackLongVector:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readInt());
			sequence_ = new Sequence_(Vector);
			sequence_.unpack_(dstr, size);
			break;
		}
		case PackShortList:{
			type_ = Undefined;
			int size = dstr.readUnsignedByte();
			sequence_ = new Sequence_(List);
			sequence_.unpack_(dstr, size);
			break;
		}
		case List:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readUnsignedShort());
			sequence_ = new Sequence_(List);
			sequence_.unpack_(dstr, size);
			break;
		}
		case PackLongList:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readInt());
			sequence_ = new Sequence_(List);
			sequence_.unpack_(dstr, size);
			break;
		}
		case PackShortQueue:{
			type_ = Undefined;
			int size = dstr.readUnsignedByte();
			sequence_ = new Sequence_(Queue);
			sequence_.unpack_(dstr, size);
			break;
		}
		case Queue:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readUnsignedShort());
			sequence_ = new Sequence_(Queue);
			sequence_.unpack_(dstr, size);
			break;
		}
		case PackLongQueue:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readInt());
			sequence_ = new Sequence_(Queue);
			sequence_.unpack_(dstr, size);
			break;
		}
		case Function:{
			type_ = Function;
			int len = dstr.readUnsignedByte();

			byte[] buf = new byte[len];
			int n = dstr.read(buf, 0, len);
			assert n == len;
			head_ = new String(buf);

			int size = dstr.readUnsignedByte();
			sequence_ = new Sequence_(Vector);
			sequence_.unpack_(dstr, size);

			int mapSize = dstr.readUnsignedByte();
			map_ = new Map_(Map);
			map_.unpack_(dstr, mapSize);

			break;
		}
		case PackLongFunction:{
			type_ = Function;
			int len = reverseBytes(dstr.readInt());

			byte[] buf = new byte[len];
			int n = dstr.read(buf, 0, len);
			assert n == len;
			head_ = new String(buf);

			int size = reverseBytes(dstr.readInt());
			sequence_ = new Sequence_(Vector);
			sequence_.unpack_(dstr, size);

			int mapSize = reverseBytes(dstr.readInt());
			map_ = new Map_(Map);
			map_.unpack_(dstr, mapSize);

			break;
		}
		case HeadSequence:{
			type_ = HeadSequence;
			unpack_(dstr);

			nvar s = new nvar();
			s.unpack_(dstr);
			sequence_ = s.sequence_;
			
			break;
		}
		case PackShortSet:{
			type_ = Undefined;
			int size = dstr.readUnsignedByte();
			map_ = new Map_(Set);
			map_.unpack_(dstr, size);
			break;
		}
		case Set:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readUnsignedShort());
			map_ = new Map_(Set);
			map_.unpack_(dstr, size);
			break;
		}
		case PackLongSet:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readInt());
			map_ = new Map_(Set);
			map_.unpack_(dstr, size);
			break;
		}
		case PackShortHashSet:{
			type_ = Undefined;
			int size = dstr.readUnsignedByte();
			map_ = new Map_(HashSet);
			map_.unpack_(dstr, size);
			break;
		}
		case HashSet:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readUnsignedShort());
			map_ = new Map_(HashSet);
			map_.unpack_(dstr, size);
			break;
		}
		case PackLongHashSet:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readInt());
			map_ = new Map_(HashSet);
			map_.unpack_(dstr, size);
			break;
		}
		case PackShortMap:{
			type_ = Undefined;
			int size = dstr.readUnsignedByte();
			map_ = new Map_(Map);
			map_.unpack_(dstr, size);
			break;
		}
		case Map:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readUnsignedShort());
			map_ = new Map_(Map);
			map_.unpack_(dstr, size);
			break;
		}
		case PackLongMap:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readInt());
			map_ = new Map_(Map);
			map_.unpack_(dstr, size);
			break;
		}
		case PackShortHashMap:{
			type_ = Undefined;
			int size = dstr.readUnsignedByte();
			map_ = new Map_(HashMap);
			map_.unpack_(dstr, size);
			break;
		}
		case HashMap:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readUnsignedShort());
			map_ = new Map_(HashMap);
			map_.unpack_(dstr, size);
			break;
		}
		case PackLongHashMap:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readInt());
			map_ = new Map_(HashMap);
			map_.unpack_(dstr, size);
			break;
		}
		case PackShortMultimap:{
			type_ = Undefined;
			int size = dstr.readUnsignedByte();
			map_ = new Map_(Multimap);
			map_.unpack_(dstr, size);
			break;
		}
		case Multimap:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readUnsignedShort());
			map_ = new Map_(Multimap);
			map_.unpack_(dstr, size);
			break;
		}
		case PackLongMultimap:{
			type_ = Undefined;
			int size = reverseBytes(dstr.readInt());
			map_ = new Map_(Multimap);
			map_.unpack_(dstr, size);
			break;
		}
		case HeadMap:{
			type_ = HeadMap;
			unpack_(dstr);

			nvar m = new nvar();
			m.unpack_(dstr);
			map_ = m.map_;
			
			break;
		}
		case SequenceMap:{
			type_ = SequenceMap;

			nvar s = new nvar();
			s.unpack_(dstr);
			sequence_ = s.sequence_;

			nvar m = new nvar();
			m.unpack_(dstr);
			map_ = m.map_;
			
			break;
		}
		case HeadSequenceMap:{
			type_ = HeadSequenceMap;
			unpack_(dstr);

			nvar s = new nvar();
			s.unpack_(dstr);
			sequence_ = s.sequence_;

			nvar m = new nvar();
			m.unpack_(dstr);
			map_ = m.map_;
			
			break;
		}
		default:
			assert false;
		}
	}

	static void out(String str){
		System.out.println(str);
	}

	private char type_;
	private Object head_;

	private Sequence_ sequence_;
	private Map_ map_;
}
