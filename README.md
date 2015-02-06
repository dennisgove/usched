1. Description

  uSched services provide an interface to schedule commands to be executed at a particular time, optionally repeating them over a specificied interval, and optionally stopping them at any other particular time.

  It provides a simple and intuitive structured language that is intepreted via a command line client, but can also be integrated into any programming language through its client libraries and bindings.

  It also operates as a client/server, where requests performed by clients can affect local or remote machines where uSched services are running.



2. Installation

  # cd /usr/src
  # mkdir usched
  # wget https://github.com/ucodev/usched/archive/master.zip
  # unzip -d usched master.zip
  # cd usched
  # ./deploy

  or

  See INSTALL.txt



3. Command-Line Usage Examples

  Run the do_backups.sh script at 23:00 and then run it every 24 hours:

      $ usc run '/usr/local/bin/do_backups.sh' on hour 23 then every 24 hours


  Dump 'df -h' output into /tmp/disk_stats.log after 10 minutes of running this command, and run it again every 30 minutes:

      $ usc run '/bin/df -h >> /tmp/disk_stats.log' in 10 minutes then every 30 minutes


  Run the command 'sync' now, repeat every 45 seconds and stop when the time is 12:00:

      $ usc run '/bin/sync' now then every 45 seconds until to time '12:00:00'


  Show all scheduled entries for the user by running the following command:

      $ usc show all


  Stop all scheduled entries for the user by running the following command:

      $ usc stop all



4. Library Usage Examples

  See example/*



5. Documentation

  See doc/*



6. Notes

  This project is on a beta stage.
