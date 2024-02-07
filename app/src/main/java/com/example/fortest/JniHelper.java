package com.example.fortest;
public class JniHelper {
    static {
        System.loadLibrary("fortest");
    }

    public static native void SDK_init();
    public static native void KPL_initialize(String mode, StationParameters stationParameters, double cut, double intv);

    public static native void sendLoadStorage(String targetPath);

    public static native void sendEphData(byte[] receiveByte);

    public static native void sendSsrData(byte[] bytes);

    public static native void sendObsData(byte[] bytes);

    public static native String SDKRetrieve(String type,  int len);
}

class StationParameters {
    public String name;
    public String marker;
    public String antdes;
    public String antsno;
    public String rectype;
    public String recver;
    public String recsno;
    public int antsetup;
    public int itrf;
    public int deltype;
    public double[] pos;
    public double[] del;
    public double hgt;
    public int glo_cp_align;
    public double[] glo_cp_bias;

    public StationParameters() {
        this.name = "";
        this.marker = "";
        this.antdes = "";
        this.antsno = "";
        this.rectype = "";
        this.recver = "";
        this.recsno = "";
        this.antsetup = 0;
        this.itrf = 0;
        this.deltype = 0;
        this.pos = new double[3]; // Assuming pos is a 3-dimensional array
        this.del = new double[3]; // Assuming del is a 3-dimensional array
        this.hgt = 0.0;
        this.glo_cp_align = 0;
        this.glo_cp_bias = new double[3]; // Assuming glo_cp_bias is a 3-dimensional array
    }
}