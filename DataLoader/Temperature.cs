using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DataLoader
{
    public class Temperature
    {
        public int ID { get; set; }
        public DateTime Time { get; set; }
        public byte Point { get; set; }
        public double Value { get; set; }
    }
}
