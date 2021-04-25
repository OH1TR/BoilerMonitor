using BoilerMonitorServer.DBContext;
using Microsoft.AspNetCore.Mvc;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace BoilerMonitorServer.Controllers
{
    [Route("api/[controller]")]
    public class ValuesController : Controller
    {
        private readonly BoilerContext _context;

        public ValuesController(BoilerContext context)
        {
            _context = context;
        }

        // POST api/values
        [HttpPost]
        public void Post([FromBody] Temperature value)
        {
            if (_context.Temperatures.Where(i => i.Point == value.Point && i.Time == value.Time).Count() == 0)
                _context.Temperatures.Add(value);

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
    }
}
