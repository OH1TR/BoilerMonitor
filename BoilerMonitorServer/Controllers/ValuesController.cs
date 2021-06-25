using BoilerMonitorServer.DBContext;
using DTO;
using Microsoft.AspNetCore.Mvc;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace BoilerMonitorServer.Controllers
{
    [ApiController]
    [Route("[controller]")]
    public class ValuesController : ControllerBase
    {
        private readonly BoilerContext _context;

        public ValuesController(BoilerContext context)
        {
            _context = context;
        }

        // POST api/values
        [HttpPost]
        public void Post([FromBody] Temperature[] values)
        {
            foreach (var value in values)
            {
                if (_context.Temperatures.Where(i => i.Point == value.Point && i.Time == value.Time).Count() == 0)
                    _context.Temperatures.Add(value);
            }
            _context.SaveChanges();
        }
        [HttpGet("latest")]
        public LatestValues Latest()
        {
            LatestValues retval = new LatestValues() { Time = DateTime.MaxValue };

            var val1 = _context.Temperatures.Where(i => i.Point == 0).OrderByDescending(i => i.Time).Take(1).FirstOrDefault();
            var val2 = _context.Temperatures.Where(i => i.Point == 2).OrderByDescending(i => i.Time).Take(1).FirstOrDefault();
            var smoke = _context.Temperatures.Where(i => i.Point == 3).OrderByDescending(i => i.Time).Take(1).FirstOrDefault();

            if (val1 != null)
            {
                retval.Temp1 = val1.Value;
                retval.Time = retval.Time > val1.Time ? val1.Time : retval.Time;
            }

            if (val2 != null)
            {
                retval.Temp2 = val2.Value;
                retval.Time = retval.Time > val2.Time ? val2.Time : retval.Time;
            }

            if (smoke != null)
            {
                retval.Smoke = smoke.Value;
                retval.Time = retval.Time > smoke.Time ? smoke.Time : retval.Time;
            }

            return (retval);
        }

        [HttpGet]
        [Route("get12hData")]
        public IEnumerable<Trace> Get12hData()
        {

            DateTime now = DateTime.UtcNow.FloorSeconds();
            DateTime minus12 = DateTime.UtcNow.AddHours(-12);
            DateTime[] times = new DateTime[12 * 60];
            double[] values0 = new double[12 * 60];
            double[] values2 = new double[12 * 60];

            for (int i = 0; i < times.Length; i++)
                times[i] = minus12.AddMinutes(i).ToLocalTime();

            var data = _context.Temperatures.Where(i => i.Time > minus12).ToArray();
            foreach (var m in data.GroupBy(t => new { t.Time }).Select(i => new
            {
                i.Key.Time,
                Temp0 = i.Where(i => i.Point == 0).Average(p => (double?)p.Value) ?? 0,
                Temp1 = i.Where(i => i.Point == 1).Average(p => (double?)p.Value) ?? 0,
                Temp2 = i.Where(i => i.Point == 2).Average(p => (double?)p.Value) ?? 0,
                Temp3 = i.Where(i => i.Point == 3).Average(p => (double?)p.Value) ?? 0
            }))
            {
                int index = (int)((m.Time.FloorSeconds() - minus12).TotalMinutes);
                if (index > 0 && index < times.Length)
                {
                    values0[index] = m.Temp0;
                    values2[index] = m.Temp2;
                }
            }
            for (int i = 1; i < values0.Length; i++)
            {
                if (values0[i] == 0)
                    values0[i] = values0[i - 1];
            }

            for (int i = 1; i < values2.Length; i++)
            {
                if (values2[i] == 0)
                    values2[i] = values2[i - 1];
            }

            var result = new Trace[] { new Trace() { x = times, y = values0, name = "Trace0" }, new Trace() { x = times, y = values2, name = "Trace2" } };
            return result;
        }
    }
}
