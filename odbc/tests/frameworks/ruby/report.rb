require 'json'
variant = ARGV.shift
require variant
connection = ARGV.shift
results = []
$dsn, $uid, $pwd = connection, nil, nil
Dir.glob('[0-9]*.rb').sort.each do |path|
  begin
    load path
    results << {name: path, status: 'passed'}
  rescue Exception => error
    results << {name: path, status: 'failed', message: "#{error.class}: #{error.message}\n#{error.backtrace.join("\n")}"}
    break
  end
end
File.write(File.join(ENV.fetch('ODBC_NATIVE_RESULTS_DIR'),
                    variant + '.json'), JSON.generate(results))
begin
  if $c && $c.connected?
    $c.drop_all
    $c.do('DROP TABLE IF EXISTS test')
    $c.disconnect
  end
rescue => error
  warn error.message
end
