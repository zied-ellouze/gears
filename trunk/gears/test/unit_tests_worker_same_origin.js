var wp = google.gears.workerPool;
// We purposely do NOT call allowCrossOrigin() in the same-origin worker.

wp.onmessage = function(body, sender) {

  // Do something to prove the database is functional.
  //
  // To prove it here, we create a table named after the message body,
  // insert our reply message, and retrieve our reply from that table.
  //
  // We also use the message body to determine the table name.  This allows
  // callers to affect the table name, so they can create different tables
  // in different scenarios to test different conditions.

  var tableName = body.split(' ')[0];

  var db = google.gears.factory.create('beta.database', '1.0');

  db.open('worker_js');
  db.execute('create table if not exists ' + tableName +
             ' (REPLY text, ID int unique)');
  db.execute('insert or replace into ' + tableName + ' values (?, ?)',
             ['RE: ' + body, sender]);
  var rs = db.execute('select * from ' + tableName + ' where ID=?', [sender]);
  var reply = rs.field(0);
  rs.close()
  db.close();

  wp.sendMessage(reply, sender);  // echo
}
