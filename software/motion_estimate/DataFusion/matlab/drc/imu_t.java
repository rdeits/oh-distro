/* LCM type definition class file
 * This file was automatically generated by lcm-gen
 * DO NOT MODIFY BY HAND!!!!
 */

package drc;
 
import java.io.*;
import java.util.*;
import lcm.lcm.*;
 
public final class imu_t implements lcm.lcm.LCMEncodable
{
    public long utime;
    public String frame_id;
    public double orientation[];
    public double orientation_covariance[];
    public double angular_velocity[];
    public double angular_velocity_covariance[];
    public double linear_acceleration[];
    public double linear_acceleration_covariance[];
 
    public imu_t()
    {
        orientation = new double[4];
        orientation_covariance = new double[9];
        angular_velocity = new double[3];
        angular_velocity_covariance = new double[9];
        linear_acceleration = new double[3];
        linear_acceleration_covariance = new double[9];
    }
 
    public static final long LCM_FINGERPRINT;
    public static final long LCM_FINGERPRINT_BASE = 0x0379ab2617c43019L;
 
    static {
        LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class<?>>());
    }
 
    public static long _hashRecursive(ArrayList<Class<?>> classes)
    {
        if (classes.contains(drc.imu_t.class))
            return 0L;
 
        classes.add(drc.imu_t.class);
        long hash = LCM_FINGERPRINT_BASE
            ;
        classes.remove(classes.size() - 1);
        return (hash<<1) + ((hash>>63)&1);
    }
 
    public void encode(DataOutput outs) throws IOException
    {
        outs.writeLong(LCM_FINGERPRINT);
        _encodeRecursive(outs);
    }
 
    public void _encodeRecursive(DataOutput outs) throws IOException
    {
        char[] __strbuf = null;
        outs.writeLong(this.utime); 
 
        __strbuf = new char[this.frame_id.length()]; this.frame_id.getChars(0, this.frame_id.length(), __strbuf, 0); outs.writeInt(__strbuf.length+1); for (int _i = 0; _i < __strbuf.length; _i++) outs.write(__strbuf[_i]); outs.writeByte(0); 
 
        for (int a = 0; a < 4; a++) {
            outs.writeDouble(this.orientation[a]); 
        }
 
        for (int a = 0; a < 9; a++) {
            outs.writeDouble(this.orientation_covariance[a]); 
        }
 
        for (int a = 0; a < 3; a++) {
            outs.writeDouble(this.angular_velocity[a]); 
        }
 
        for (int a = 0; a < 9; a++) {
            outs.writeDouble(this.angular_velocity_covariance[a]); 
        }
 
        for (int a = 0; a < 3; a++) {
            outs.writeDouble(this.linear_acceleration[a]); 
        }
 
        for (int a = 0; a < 9; a++) {
            outs.writeDouble(this.linear_acceleration_covariance[a]); 
        }
 
    }
 
    public imu_t(byte[] data) throws IOException
    {
        this(new LCMDataInputStream(data));
    }
 
    public imu_t(DataInput ins) throws IOException
    {
        if (ins.readLong() != LCM_FINGERPRINT)
            throw new IOException("LCM Decode error: bad fingerprint");
 
        _decodeRecursive(ins);
    }
 
    public static drc.imu_t _decodeRecursiveFactory(DataInput ins) throws IOException
    {
        drc.imu_t o = new drc.imu_t();
        o._decodeRecursive(ins);
        return o;
    }
 
    public void _decodeRecursive(DataInput ins) throws IOException
    {
        char[] __strbuf = null;
        this.utime = ins.readLong();
 
        __strbuf = new char[ins.readInt()-1]; for (int _i = 0; _i < __strbuf.length; _i++) __strbuf[_i] = (char) (ins.readByte()&0xff); ins.readByte(); this.frame_id = new String(__strbuf);
 
        this.orientation = new double[(int) 4];
        for (int a = 0; a < 4; a++) {
            this.orientation[a] = ins.readDouble();
        }
 
        this.orientation_covariance = new double[(int) 9];
        for (int a = 0; a < 9; a++) {
            this.orientation_covariance[a] = ins.readDouble();
        }
 
        this.angular_velocity = new double[(int) 3];
        for (int a = 0; a < 3; a++) {
            this.angular_velocity[a] = ins.readDouble();
        }
 
        this.angular_velocity_covariance = new double[(int) 9];
        for (int a = 0; a < 9; a++) {
            this.angular_velocity_covariance[a] = ins.readDouble();
        }
 
        this.linear_acceleration = new double[(int) 3];
        for (int a = 0; a < 3; a++) {
            this.linear_acceleration[a] = ins.readDouble();
        }
 
        this.linear_acceleration_covariance = new double[(int) 9];
        for (int a = 0; a < 9; a++) {
            this.linear_acceleration_covariance[a] = ins.readDouble();
        }
 
    }
 
    public drc.imu_t copy()
    {
        drc.imu_t outobj = new drc.imu_t();
        outobj.utime = this.utime;
 
        outobj.frame_id = this.frame_id;
 
        outobj.orientation = new double[(int) 4];
        System.arraycopy(this.orientation, 0, outobj.orientation, 0, 4); 
        outobj.orientation_covariance = new double[(int) 9];
        System.arraycopy(this.orientation_covariance, 0, outobj.orientation_covariance, 0, 9); 
        outobj.angular_velocity = new double[(int) 3];
        System.arraycopy(this.angular_velocity, 0, outobj.angular_velocity, 0, 3); 
        outobj.angular_velocity_covariance = new double[(int) 9];
        System.arraycopy(this.angular_velocity_covariance, 0, outobj.angular_velocity_covariance, 0, 9); 
        outobj.linear_acceleration = new double[(int) 3];
        System.arraycopy(this.linear_acceleration, 0, outobj.linear_acceleration, 0, 3); 
        outobj.linear_acceleration_covariance = new double[(int) 9];
        System.arraycopy(this.linear_acceleration_covariance, 0, outobj.linear_acceleration_covariance, 0, 9); 
        return outobj;
    }
 
}
