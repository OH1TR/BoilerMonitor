using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace BoilerMonitorServer.DBContext
{
    public class Temperature
    {
        public int ID { get; set; }
        public DateTime Time { get; set; }
        public byte Point { get; set; }
        public float Value { get; set; }
    }
}
